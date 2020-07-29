#include<bits/stdc++.h>
using namespace std;

vector<pair<int,int> >pageTable;
vector<pair<int,int> >tlb;

int physicalMemory[256][256] = {0};
int firstAvailableFrame=0;
int tlbHits = 0,pageFaults = 0;
double total = 0;

void matchResult()
{
	FILE *fp1 = fopen("correct.txt","r");
	FILE *fp2 = fopen("output.txt","r");

	char str1[200],str2[200];
	int i=0;
	while(fgets(str1,200,fp1) != NULL && fgets(str2,200,fp2)!= NULL)
	{
		i++;
		if(strcmp(str1,str2) != 0)
		{
			cout<<"Your answers are wrong at line "<<i<<endl;
			return;
		}
	}
	cout<<"Files matched - Your answers are correct!!"<<endl;
	fclose(fp1);
	fclose(fp2);
}

void updateTLB(int pageNo,int frameNo)
{
	int flag=0,i=0;
	for(i=0;i<tlb.size();i++)
	{
		if(tlb[i].first == pageNo)
		{
			flag = 1;
			break;
		}
	}
	if(flag == 0)
	{
		if(tlb.size()<16)
		{
			tlb.push_back(make_pair(pageNo,frameNo));
		}
		else
		{
			tlb.erase(tlb.begin());
			tlb.push_back(make_pair(pageNo,frameNo));
		}
	}
	else
	{
		tlb.erase(tlb.begin() + i);
		tlb.push_back(make_pair(pageNo,frameNo));
	}
}

void readStore(int pageNo)
{
	FILE *backing_store = fopen("backing_store.bin","rb");
	if(backing_store == NULL)
	{
		cout<<"Couldn't find backing store file"<<endl;
		exit(1);
	}
	if(fseek(backing_store,pageNo*256,SEEK_SET) != 0)
	{
		cout<<"Error getting data from backing_store"<<endl;
		exit(1);
	}
	else
	{
		signed char buffer[256];
		if(fread(buffer,sizeof(signed char),256,backing_store) == 0)
		{
			cout<<"Error reading data from backing_store"<<endl;
			exit(1);
		}

		for(int i=0;i<256;i++)
		{
			physicalMemory[firstAvailableFrame][i] = buffer[i];
		}

		if(pageTable.size() <256)
			pageTable.push_back(make_pair(pageNo,firstAvailableFrame));
		firstAvailableFrame++;
	}
	fclose(backing_store);
}

void getFrameNumber(int logical_address)
{
	int pageNo = ((logical_address & 0xFFFF)>>8);
	int offset = (logical_address & 0xFF);
	int frameNo = -1;

	for(int i=0;i<tlb.size();i++)
	{
		if(tlb[i].first == pageNo)
		{
			tlbHits++;
			frameNo = tlb[i].second;
			break;
		}
	}
	if(frameNo == -1)
	{
		for(int i=0;i<pageTable.size();i++)
		{
			if(pageTable[i].first == pageNo)
			{
				frameNo = pageTable[i].second;
				break;
			}
		}
	}
	if(frameNo == -1)
	{
		pageFaults++;
		readStore(pageNo);
		frameNo = firstAvailableFrame - 1;
	}

	updateTLB(pageNo,frameNo);
	//cout<<frameNo<<" "<<offset<<endl;
	int val = (frameNo<<8|offset);
	cout<<"Virtual Address : "<<logical_address<<" Physical Address : "<< (frameNo<<8|offset)<<" Value : "<<physicalMemory[frameNo][offset]<<endl;
	ofstream fout;
	fout.open("output.txt",std::ios_base::app);
	fout<<"Virtual address: "<<logical_address<<" Physical address: "<< (frameNo<<8|offset)<<" Value: "<<physicalMemory[frameNo][offset]<<endl;
	fout.close();
}

int main()
{
	FILE *address_file = fopen("address.txt","r");
	if(address_file == NULL)
	{
		cout<<"address.txt file not found"<<endl;
		return 0;
	}
	int logical_address;
	char str[100];
	ofstream fout;
	fout.open("output.txt", std::ofstream::out | std::ofstream::trunc);
	fout.close();
	while(fgets(str,10,address_file) != NULL)
	{
		logical_address = atoi(str);
		getFrameNumber(logical_address);
		total++;
	}
	cout<<"Number of TLB Hits : "<<tlbHits<<endl;
	cout<<"Number of Page Faults : "<<pageFaults<<endl;
	cout<<"TLB Hit Rate : "<<tlbHits/total<<endl;
	cout<<"Page Fault Rate : "<<pageFaults/total<<endl;
	matchResult();

	fclose(address_file);

}
