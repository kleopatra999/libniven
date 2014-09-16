#include "niven/Response.h"
#include "niven/Mime.h"
#include <emergent/Emergent.h>

using namespace std;
using namespace ent;


namespace niven
{
	Response::Response(const entity &data)
	{
		this->headers["Content-Type"]	= "application/json";
		this->data						= const_cast<entity &>(data).to<json>();
	}

	Response::Response(const entity2 &data)
	{
		this->headers["Content-Type"]	= "application/json";
		this->data						= encode<json2>(const_cast<entity2 &>(data));
	}


	Response::Response(const tree &data)
	{
		this->headers["Content-Type"]	= "application/json";
		this->data						= json::to(data, false);
	}


	Response::Response(const Path &path)
	{
		auto extension = path.extension();

		if (path.exists() && Mime::ByExtension.count(extension))
		{
			this->headers["Content-Type"]	= Mime::ByExtension.at(extension);
			this->data 						= emergent::load(path);
		}
		else this->status = Http::NotFound;
	}


	Response &Response::WithHeader(string key, string value)
	{
		this->headers[key] = value;
		return *this;
	}


	Response &Response::WithCookie(const Cookie &cookie)
	{
		this->cookies.push_back(cookie);
		return *this;
	}


	int Response::Send(MHD_Connection *connection)
	{
		auto response = MHD_create_response_from_data(this->data.size(), (void *)this->data.data(), MHD_NO, MHD_YES);

		for (auto &h : this->headers) MHD_add_response_header(response, h.first.c_str(), h.second.c_str());
		for (auto &c : this->cookies) MHD_add_response_header(response, "Set-Cookie", c.Build().c_str());

		MHD_add_response_header(response, "Server", "niven/0.1");
		MHD_add_response_header(response, "Keep-Alive", "timeout=60");

		//MHD_set_connection_value(struct MHD_Connection *connection, enum MHD_ValueKind kind, const char *key, const char *value)

		int result = MHD_queue_response(connection, (int)this->status, response);

		MHD_destroy_response(response);

		return result;
	}
}