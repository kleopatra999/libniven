#include "niven/routing/Router.h"
#include <emergent/Logger.h>

using namespace std;
using namespace emg;


namespace niven
{
	void Router::Initialise(map<string, unique_ptr<NivenModule>> &modules)
	{
		// Build the trie
		for (auto &m : modules)
		{
			FLOG(info, "Registering routes for module %s", m.first);

			for (auto &r : m.second->routes)
			{
				this->trie[r->method].Add(r);
			}
		}
	}


	Response Router::Resolve(Context &context)
	{
		vector<RouteMatch> matches;
		auto method = context.request.method;
		auto path	= context.request.url;

		if (!path.empty() && this->trie.count(method))
		{
			this->trie[method].GetMatches(matches, explode(path, "/"));

			if (matches.size())
			{
				auto best = max_element(matches.begin(), matches.end(), [](RouteMatch &a, RouteMatch &b) { return a.route->score < b.route->score; });
				context.parameters = best->parameters;

				try
				{
					if (best->route->action) return best->route->action(context);
				}
				catch (const std::exception &e)
				{
					return { e.what(), Http::InternalServerError };
				}

				return { "Missing action for route: " + best->route->path, Http::InternalServerError };
			}
		}

		return Http::NotFound;
	}
}
