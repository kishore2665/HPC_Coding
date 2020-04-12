#include <iostream>
#include <thread>
#include <array>
#include <vector>
#include <mutex>
#include <string>
#include <condition_variable>
#include <queue>
#include <algorithm>
#include <unordered_map>
using namespace std;

mutex m;
condition_variable producer_cv, consumer_cv;
queue<string> buffer;
unordered_map<long, string> umap;
unsigned int index = 0;
bool relConsumer = false;
int numOfadjacents = 8;

string numstring = "3766581235885941622054540050228447514162777869412307699482907769113268717216818322831603491835999456015306915009196661427591452909871214219792485776087253286386945942663949956280302377388971714236415605168862773550156548824873689737766284562457836197902674997734737908387650371844408009421100914050765521827781655182806129058522352838472989652688571683680665438395803243794489830567998343203397981373552644309879795957322883020671901669290704497751685870539575543632177623725028726840870016429503564354896057020404025619555440159796686935523081354355119387766201895202371147907112778884969266539280935452003712638970422340890791962244529017494651550289995762505866212386393472458374741386036991340760970327022447106502711257671708182087831698677130077927731626466195021513131952322762659409302452718743061757527857578831917621650745174966732316231446870605534431568974878576006012026939455247174486040603096495646182217557200423380237313587369836078574982810508277521659834594761360129982400036745363";

const int BUFFER_SIZE = 20;

//Consumer thread consumes data from Queue, which is substring of 8 adjacents
//It calculates the product and saves into undordered map.
//Once proceesed removes message from Queue.
void consumer_thread(int id)
{
	while (true)
	{
		unique_lock<mutex> lk(m);
		if (((index - 1) == numstring.size() - numOfadjacents) && buffer.empty())
		{
			lk.unlock();
			cout << "*** Consumer " << id << " is thread task completed, EXITING." << endl;
			break;
		}

		if (!buffer.empty())
		{
			long prod = 1;
			string substr = buffer.front();
			for (char num : substr)
			{
				prod *= num - '0';
			}
			umap[prod] = substr;
			buffer.pop();
			producer_cv.notify_all();
		}
		else
		{
			consumer_cv.wait(lk, [] { return !buffer.empty() || relConsumer; });
		}
		lk.unlock();
	}
}

//Producer thread produce substring based on given adjacent ie 8 and store into Queue.

void producer_thread(int id)
{
	while (true)
	{
		unique_lock<mutex> lk(m);

		if (((index - 1) == numstring.size() - numOfadjacents) && buffer.empty())
		{
			consumer_cv.notify_all();
			relConsumer = true;
			lk.unlock();
			cout << "*** Producer " << id << " is thread task completed, EXITING."<< endl;
			break;
		}

		if (buffer.size() < BUFFER_SIZE && index + numOfadjacents <= numstring.size())
		{
			string data = "";
			data = numstring.substr(index, numOfadjacents);
			buffer.push(data);
			++index;
			consumer_cv.notify_all();
		}
		else
		{
			producer_cv.wait(lk, [] { return buffer.size() < BUFFER_SIZE; });
		}

		lk.unlock();
	}
}

int main()
{
	const int consumers_count = 5;
	const int producers_count = 5;

	vector<thread> producers;
	vector<thread> consumers;

	for (int i = 0; i < consumers_count; i++)
		consumers.push_back(thread(consumer_thread, i + 1));
	for (int i = 0; i < producers_count; i++)
		producers.push_back(thread(producer_thread, i + 1));
	for (int i = 0; i < consumers_count; i++)
		consumers[i].join();
	for (int i = 0; i < producers_count; i++)
		producers[i].join();

	if (index < numstring.size() && buffer.empty())
	{
		cout << endl << endl;
		cout << "*************Finished Thread Processing.*************"<<endl;
	}

	//Trying to find maximum product from given unordered map.
	auto result = std::max_element(umap.begin(), umap.end());
	cout << endl;
	cout <<"Maximum Product:" <<result->first <<" | Sequence:"<< result->second<<endl;
	getchar();
	return 1;
}