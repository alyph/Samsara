
#include "engine/string.h"
#include <cstdlib>

int main()
{
	Engine app_engine;
	set_engine(app_engine);

	String str1 = "good man whatever ok";
	asserts(str1.str_data.is_short());

	String str2 = "It was of no use to demonstrate to such opponents that the Vermont myths differed but little in essence from those universal legends of natural personification which filled the ancient world with fauns and dryads and satyrs, suggested the kallikanzarai of modern Greece, and gave to wild Wales and Ireland their dark hints of strange, small, and terrible hidden races of troglodytes and burrowers. No use, either, to point out the even more startlingly similar belief of the Nepalese hill tribes in the dreaded Mi-Go or \"Abominable Snow-Men\" who lurk hideously amidst the ice and rock pinnacles of the Himalayan summits. When I brought up this evidence, my opponents turned it against me by claiming that it must imply some actual historicity for the ancient tales; that it must argue the real existence of some queer elder earth-race, driven to hiding after the advent and dominance of mankind, which might very conceivably have survived in reduced numbers to relatively recent times—or even to the present.";
	asserts(str2.str_data.is_normal());

	String str3 = "another fairly long string, but not as crazy";
	asserts(str3.str_data.is_normal());

	printf("first string: %s\n", str1.c_str());
	printf("second string: %s\n", str2.c_str());
	printf("third string: %s\n", str3.c_str());

	TempString temp_str = "temporary allocated string is awesome, am I right?";
	printf("temp string: %s\n", temp_str.c_str());
	asserts((temp_str.str_data.normal_data.alloc_handle.header >> 28) == 1);

	temp_str = str1;
	asserts(temp_str.str_data.is_short());
	printf("temp string is now: %s\n", temp_str.c_str());

	temp_str = str3;
	printf("temp string is long now: %s\n", temp_str.c_str());
	asserts((temp_str.str_data.normal_data.alloc_handle.header >> 28) == 1);

	{
		String str4 = str3;
		printf("forth string is : %s\n", str4.c_str());
		asserts((str4.str_data.normal_data.alloc_handle.header >> 28) == 3);
		asserts(str4.str_data.header()->ref_count == 2);
		asserts(str3.str_data.header()->ref_count == 2);

		str4 = str2;
		asserts(str3.str_data.header()->ref_count == 1);
		asserts(str4.str_data.header()->ref_count == 2);
		asserts(str2.str_data.header()->ref_count == 2);
	}

	asserts(str2.str_data.header()->ref_count == 1);

	std::system("pause");

	return 0;
}