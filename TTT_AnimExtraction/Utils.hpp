#include <filesystem>

template<typename T>
bool endsWith(const T& str, const T& suffix)
{
	if (str.length() < suffix.length())
		return false;
	size_t i = str.length() - suffix.length();

	for (auto c : suffix)
	{
		if (c != str[i]) {
			return false;
		}
		++i;
	}

	return true;
}
