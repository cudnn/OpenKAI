#include "JSON.h"

namespace kai
{

JSON::JSON(void)
{
}

JSON::~JSON(void)
{
}

bool JSON::parse(string json)
{
	string error;
	const char* jsonstr = json.c_str();
	picojson::parse(m_JSON, jsonstr, jsonstr + strlen(jsonstr), &error);

	if (!m_JSON.is<object>())
		return false;

	return true;
}

bool JSON::v(string* pName, int* val)
{
	if (!m_JSON.is<object>())
		return false;

	value var = m_JSON.get(*pName);
	if (!var.is<int>())
		return false;

	*val = (int) var.get<double>();
	return true;
}

bool JSON::v(string* pName, bool* val)
{
	if (!m_JSON.is<object>())
		return false;

	value var = m_JSON.get(*pName);
	if (!var.is<int>())
		return false;

	*val = (((int) var.get<double>()) == 1 ? true : false);
	return true;
}

bool JSON::v(string* pName, uint64_t* val)
{
	if (!m_JSON.is<object>())
		return false;

	value var = m_JSON.get(*pName);
	if (!var.is<double>())
		return false;

	*val = (uint64_t) var.get<double>();
	return true;
}

bool JSON::v(string* pName, double* val)
{
	if (!m_JSON.is<object>())
		return false;

	value var = m_JSON.get(*pName);
	if (!var.is<double>())
		return false;

	*val = var.get<double>();
	return true;
}

bool JSON::v(string* pName, string* val)
{
	if (!m_JSON.is<object>())
		return false;

	value var = m_JSON.get(*pName);
	if (!var.is<string>())
		return false;

	*val = var.get<string>();
	return true;
}

bool JSON::array(string* pName, value::array* val)
{
	if (!m_JSON.is<object>())
		return false;

	value var = m_JSON.get(*pName);
	if (!var.is<value::array>())
		return false;

	*val = var.get<value::array>();
	return true;

	//TODO
	/*      array arr = m_JSON.get<array>();
	 array::iterator it;
	 for (it = arr.begin(); it != arr.end(); it++) {
	 object obj = it->get<object>();
	 cout << obj["user_login_id"].to_str() << ": " << obj["text"].to_str() << endl;
	 }

	 return false;
	 */
}

int JSON::checkErrorNum(void)
{
	int numError;
	string errorDesc;
	string err = "error";
	if (!v(&err, &numError))
	{
		return -1;
	}

	//TODO record error description

	return numError;
}

}
