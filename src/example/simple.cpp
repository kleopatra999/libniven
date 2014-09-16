#include "niven/Niven.h"
#include <signal.h>
#include <bits/signum.h>
#include <iostream>
#include <mutex>
#include <condition_variable>

#include <entity/json.h>

using namespace std;
using namespace emg;
using namespace ent;
using namespace niven;


#include <emergent/image/Image.h>

class ImageResource
{
	public:

		void Set(Image<byte> &image)
		{
			lock_guard<mutex> lock(this->cs);

			this->image = image;
		}


		Buffer<byte> Get()
		{
			lock_guard<mutex> lock(this->cs);

			if (this->image.Save(this->buffer, 2))
			{
				return this->buffer;
			}

			return {};
		}

	private:

		mutex cs;
		Image<byte> image;
		Buffer<byte> buffer;

};

class IResource
{
	public:
		virtual string Get() = 0;
};


class ConcreteResource : public IResource
{
	public:
		virtual string Get() { return "ConcreteResource"; }
};

class AnotherResource  : public IResource
{
	public:

		int number;

		AnotherResource(int needsNumber) : number(needsNumber) {}
		virtual string Get() { return "AnotherResource"; }
};


struct Test : entity
{
	string name = "Test";
	vector<int> list = { 1, 2, 3 };

	mapping map()
	{
		return mapping() << eref(name) << eref(list);
	}
};

#include <entity/json2.h>

struct Test2 : entity2
{
	string name = "Test";
	vector<int> list = { 1, 2, 3 };

	mapping2 map()
	{
		return mapping2() << eref2(name) << eref2(list);
	}
};

class SampleModule : public NivenModule
{
	public:

		SampleModule() : NivenModule("/sample")
		{
			//Before	+= [](Context &c) { cout << "Before module" << endl; return nullptr; };
			//After	+= [](Context &c, Response &r) { cout << "After module" << endl; };

			Get["/"]			= [](Context &c) { return "The moon's a balloon"; };
			Get["/test"]		= [](Context &c) { return Test(); };
			Get["/test2"]		= [](Context &c) { return Test2(); };
			Get["/tree"]		= [](Context &c) { return tree().set("name", "Test").set("list", vector<int>({ 1, 2, 3 })); };	// Equivalent of anonymous object in C#
			Get["/resource"]	= [](Context &c) { return c.Resolve<IResource>()->Get(); };
			Get["/ok"]			= [](Context &c) { return Http::OK; };
			Get["/custom"]		= [](Context &c) { return Response().WithHeader("something", "else"); };
			Get["/test/{id}"]	= [](Context &c) { return "ID=" + c["id"]; };
			Get["/broken"]		= nullptr;

			Get["/cookie"]		= [](Context &c) { return Response().WithCookie({"test", "value", DateTime::Now().AddSeconds(30)}); };

			Get["/test/{id}/{name}"]	= [](Context &c) { return "ID=" + c["id"] + "  Name=" + c["name"]; };
			Get["/test/{id}/literal"]	= [](Context &c) { return "ID=" + c["id"]; };
			Get["/file/{name*}"]		= [](Context &c) { return Path("/home/dan/temp") / c["name"]; };
			Get["/file/{name*}/bar"]	= [](Context &c) { return c["name"]; };

			Post["/bind"] = [](Context &c) { return "The name is: " + c.bind<Test>().name; };
		}
};

REGISTER_MODULE(SampleModule)


bool run = true;
condition_variable condition;

int main(int argc, char **argv)
{
	/*{
		auto start 	= steady_clock::now();

		for (int i=0; i<1000000; i++) uuid().to_string();

		long duration = duration_cast<milliseconds>(steady_clock::now() - start).count();
		cout << "Time taken was " << duration << "ms" << endl;
	}*/
	/*{
		auto start = steady_clock::now();
		uuid u;
		string data = encode64(u.to_binary());

		for (int i=0; i<1000000; i++) decode64(data); //encode64(u.to_binary());

		long duration = duration_cast<milliseconds>(steady_clock::now() - start).count();
		cout << "Time taken was " << duration << "ms" << endl;
	}*/

	//cout << uuid().to_string() << endl;
	//cout << encode64(uuid().to_binary()) << endl;
	//cout << uuid64() << endl;

	//return 0;

	/*auto resource = shared_ptr<IResource>(new ConcreteResource());

	cout << "Singleton" << endl;
	cout << "  Count = " << resource.use_count() << endl;
	{
		Dependencies dep;
		dep.Register<IResource>(resource);

		cout << "  Count = " << resource.use_count() << endl;

		{
			vector<shared_ptr<IResource>> resources;

			for (int i=0; i<10; i++)
			{
				resources.push_back(dep.Resolve<IResource>());

				cout << "  " << (long)resources[i].get() << endl;
				cout << "  Count = " << resource.use_count() << endl;
			}
		}

		cout << "  Count = " << resource.use_count() << endl;
	}
	cout << "  Count = " << resource.use_count() << endl;

	cout << endl << "Multiple" << endl;
	{
		Dependencies dep;

		dep.Register<IResource, ConcreteResource>();

		vector<shared_ptr<IResource>> resources;

		for (int i=0; i<10; i++)
		{
			resources.push_back(move(dep.Resolve<IResource>()));

			cout << "  " << (long)resources[i].get() << endl;
			cout << "  Count = " << resources[i].use_count() << endl;

			//cout << ((AnotherResource *)resources[i].get())->number << endl;
		}
	}

	return 0;
	//*/

	//cout << Cookie("key", "test", DateTime::Now().AddHours(1)).Build() << endl;
	//cout << DateTime::Today().FormatISO() << endl;

	//return 0;

	logger::instance().add(new sink::console());
	logger::instance().set_verbosity("info");
	signal(SIGINT,	[](int) { run = false; condition.notify_one(); });
	signal(SIGQUIT,	[](int) { run = false; condition.notify_one(); });

	mutex m;
	NivenHost host;

	//host.Configure()...

	host.Register<IResource, ConcreteResource>();
	//host.Register<IResource, AnotherResource>(42);

	//host.BeforeRequest	+= [](Context &c) { cout << "Before routing" << endl; return nullptr; };
	//host.AfterRequest	+= [](Context &c, Response &r) { cout << "The end" << endl; };

	if (host.Run(8090))
	{
		cout << "Listening on port 8090" << endl;

		unique_lock<mutex> lock(m);
		while (run) condition.wait(lock);

		host.Stop();
	}
	else cout << "Failed to initialise host" << endl;

	return 0;
}
