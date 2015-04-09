#include "niven/Configuration.h"
#include <emergent/Emergent.h>
#include <emergent/Logger.h>


using namespace std;
using namespace emergent;


namespace niven
{

	Configuration &Configuration::Listen(int port)
	{
		this->port = port;
		return *this;
	}


	Configuration &Configuration::WithThreads(int poolSize)
	{
		this->threads = poolSize;
		return *this;
	}


	Configuration &Configuration::EnableSSL(const std::string &key, const std::string &certificate)
	{
		this->ssl 				= true;
		this->sslKey			= key;
		this->sslCertificate	= certificate;
		return *this;
	}


	Configuration &Configuration::WithKey(const Path key)
	{
		if (key.exists())
		{
			this->sslKey = String::load(key);
		}
		else FLOG(error, "Path to key file does not exist: %s", key);

		return *this;
	}


	Configuration &Configuration::WithCertificate(const Path certificate)
	{
		if (certificate.exists())
		{
			this->sslCertificate = String::load(certificate);
		}
		else FLOG(error, "Path to certificate file does not exist: %s", certificate);

		return *this;
	}
}