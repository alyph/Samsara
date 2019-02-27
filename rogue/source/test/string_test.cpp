
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

	TempString temp_str = "temporarily allocated string is awesome, am I right?";
	printf("temp string: %s\n", temp_str.c_str());
	asserts((temp_str.str_data.normal_data.alloc_handle.header >> 28) == 1);

	temp_str = str1;
	asserts(temp_str.str_data.is_short());
	printf("temp string is now: %s\n", temp_str.c_str());

	temp_str = str3;
	asserts(temp_str.str_data.is_normal());
	printf("temp string is long now: %s\n", temp_str.c_str());
	asserts((temp_str.str_data.normal_data.alloc_handle.header >> 28) == 1);

	StringData str4_data;
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

		str2 = "second string gets asigned to new one";
		asserts(str4.str_data.header()->ref_count == 1);
		asserts(str2.str_data.header()->ref_count == 1);
		printf("second string now : %s\n", str2.c_str());
		printf("forth string now : %s\n", str4.c_str());

		// str4 will be deallocated, however it is still safe to access its data
		// since deallocation actually doesn't do anything until the allocator shrinks
		str4_data = str4.str_data;
	}

	// although we can still access this data, but we can confirm the ref count is now 0 
	// and the memory can go away when the allocator shrinks
	asserts(str4_data.header()->ref_count == 0);

	String reloc_str1 = "first long enough string to relocate actually";
	asserts(reloc_str1.str_data.is_normal());

	String reloc_str2 = "second really long string to relocate for sure, which blocks the first string";
	
	auto old_ptr2 = reloc_str2.str_data.normal_data.alloc_handle.get(engine().allocators);
	auto old_len2 = reloc_str2.size();
	reloc_str2 = "second really long string to relocate for sure, which blocks the first string + relocated portion";
	auto new_ptr2 = reloc_str2.str_data.normal_data.alloc_handle.get(engine().allocators);
	auto new_len2 = reloc_str2.size();
	asserts(old_ptr2 == new_ptr2);

	auto old_ptr1 = reloc_str1.str_data.normal_data.alloc_handle.get(engine().allocators);
	auto old_len1 = reloc_str1.size();
	reloc_str1 = "first long enough string to relocate actually -- need more";
	auto new_ptr1 = reloc_str1.str_data.normal_data.alloc_handle.get(engine().allocators);
	auto new_len1 = reloc_str1.size();

	printf("relocated first string : %s\n", reloc_str1.c_str());
	printf("relocated second string : %s\n", reloc_str2.c_str());

	asserts(new_ptr1 == reinterpret_cast<uint8_t*>(new_ptr2) + reloc_str2.str_data.normal_data.alloc_handle.capacity(engine().allocators));
	
	old_ptr2 = new_ptr2;
	reloc_str2 = "second really long string to relocate for sure, which blocks the first string + relocated portion and a little more";
	new_ptr2 = reloc_str2.str_data.normal_data.alloc_handle.get(engine().allocators);
	asserts(old_ptr2 == new_ptr2);
	printf("relocated second string now : %s\n", reloc_str2.c_str());

	reloc_str2 = "second really long string to relocate for sure, which blocks the first string + relocated portion and a little more double up: second really long string to relocate for sure, which blocks the first string + relocated portion and a little more";
	new_ptr2 = reloc_str2.str_data.normal_data.alloc_handle.get(engine().allocators);
	asserts(old_ptr2 != new_ptr2);
	printf("relocated second string final : %s\n", reloc_str2.c_str());


	std::system("pause");

	return 0;
}