
#include "engine/string.h"
#include <cstdlib>

int main()
{
	printf("String Test Begin...\n");
	
	scoped_engine_init();

	scoped_context_allocator(Allocator::app);


	String str1 = "good man whatever ok";
	asserts(str1.str_data.is_short());
	asserts(str1.size() == 20);

	String str2 = cstr_to_str("It was of no use to demonstrate to such opponents that the Vermont myths differed but little in essence from those universal legends of natural personification which filled the ancient world with fauns and dryads and satyrs, suggested the kallikanzarai of modern Greece, and gave to wild Wales and Ireland their dark hints of strange, small, and terrible hidden races of troglodytes and burrowers. No use, either, to point out the even more startlingly similar belief of the Nepalese hill tribes in the dreaded Mi-Go or \"Abominable Snow-Men\" who lurk hideously amidst the ice and rock pinnacles of the Himalayan summits. When I brought up this evidence, my opponents turned it against me by claiming that it must imply some actual historicity for the ancient tales; that it must argue the real existence of some queer elder earth-race, driven to hiding after the advent and dominance of mankind, which might very conceivably have survived in reduced numbers to relatively recent times—or even to the present.");
	asserts(str2.str_data.is_normal());

	String str3 = cstr_to_str("another fairly long string, but not as crazy");
	asserts(str3.str_data.is_normal());

	String str9 = "string literal is normally stored as simple pointer and size";
	asserts(str9.str_data.is_literal());

	printf("first string: %s\n", str1.c_str());
	printf("second string: %s\n", str2.c_str());
	printf("third string: %s\n", str3.c_str());
	printf("literal string: %s\n", str9.c_str());

	String temp_str = cstr_to_str("temporarily allocated string is awesome, am I right?");
	printf("temp string: %s\n", temp_str.c_str());
	asserts(temp_str.str_data.normal_data.alloc_handle.allocator_type() == engine().allocators.current_temp_allocator);

	temp_str = str1;
	asserts(temp_str.str_data.is_short());
	printf("temp string is now: %s\n", temp_str.c_str());

	temp_str = str3;
	asserts(temp_str.str_data.is_normal());
	printf("temp string is long now: %s\n", temp_str.c_str());
	asserts(temp_str.str_data.normal_data.alloc_handle.allocator_type() == engine().allocators.current_temp_allocator);

#if STRING_VIEW
	StringView str4_view;
#else
	String str4_view;
#endif
	{
		str3.store();
		String str4 = str3;
		printf("forth string is : %s\n", str4.c_str());
		asserts(str4.str_data.normal_data.alloc_handle.allocator_type() == Allocator::app);
#if STRING_REF_COUNT
		asserts(string_ref_count(str4.str_data) == 2);
		asserts(string_ref_count(str3.str_data) == 2);
#endif

		str2.store();
		str4 = str2;
#if STRING_REF_COUNT
		asserts(string_ref_count(str3.str_data) == 1);
		asserts(string_ref_count(str4.str_data) == 2);
		asserts(string_ref_count(str2.str_data) == 2);
#endif

		str2 = cstr_to_str("second string gets asigned to new one");
#if STRING_REF_COUNT
		asserts(string_ref_count(str4.str_data) == 1);
#endif
		asserts(str2.str_data.normal_data.alloc_handle.allocator_type() == engine().allocators.current_temp_allocator);
		printf("second string now : %s\n", str2.c_str());
		printf("forth string now : %s\n", str4.c_str());

		String str5 = str4;
		asserts(str5.str_data.normal_data.alloc_handle.allocator_type() == Allocator::app);
#if STRING_REF_COUNT
		asserts(string_ref_count(str5.str_data) == 2);
		asserts(string_ref_count(str4.str_data) == 2);
#endif
		asserts(validate_and_get_string_buffer(str5.str_data) == validate_and_get_string_buffer(str4.str_data));

		// str4 will be deallocated, however it is still safe to access its data
		// since deallocation actually doesn't do anything until the allocator shrinks
		str4_view = str4;
	}

#if STRING_REF_COUNT
	// although we can still access this data, but we can confirm the ref count is now 0 
	// and the memory can go away when the allocator shrinks
	asserts(string_ref_count(str4_view.str_data) == 0);
#endif

	String reloc_str = cstr_to_str("first long enough string to relocate actually");
	reloc_str.store();
	asserts(reloc_str.str_data.is_normal());
	asserts(reloc_str.str_data.normal_data.alloc_handle.allocator_type() == Allocator::app);

	auto old_ptr = validate_and_get_string_buffer(reloc_str.str_data);
	auto old_size = reloc_str.size();

#if STRING_VIEW
	StringView reloc_str_view = reloc_str;
#else
	String reloc_str_view = reloc_str;
#endif

	reloc_str.store(cstr_to_str("first long enough string to relocate actually -- need more"));

	auto new_ptr = validate_and_get_string_buffer(reloc_str.str_data);

	printf("before relocation: %s\n", reloc_str_view.c_str());
	printf("after relocation: %s\n", reloc_str.c_str());

	asserts(validate_and_get_string_buffer(reloc_str_view.str_data) == old_ptr);
	asserts(reloc_str_view.size() == old_size);
	asserts(new_ptr == reinterpret_cast<uint8_t*>(old_ptr) + reloc_str_view.str_data.normal_data.alloc_handle.capacity(engine().allocators));
	
	StringBuilder builder1;
	builder1.append_format(str4_view);
	builder1.append_format(str4_view);
	String built_str = builder1.to_str();
	asserts(built_str.size() == str4_view.size() * 2);
	printf("built string: %s\n", built_str.c_str());

	StringBuilder builder2;
	builder2.append_format("123456"); // short one
	builder2.append_format("long enough to push the string builder to heap"); // long one
	String built_str2 = builder2.to_str();
	asserts(built_str2.str_data.layout() == StringLayout::normal_string);
	asserts(built_str2.size() == 52);
	printf("built string2: %s\n", built_str2.c_str());

	const auto fmt_str_short = format_str("123456%s", "78901234567890abc");
	asserts(fmt_str_short.str_data.layout() == StringLayout::short_string);
	asserts(fmt_str_short.size() == max_short_string_size);

	const auto fmt_str_normal = format_str("123456%s", "78901234567890abcdefghij");
	asserts(fmt_str_normal.str_data.layout() == StringLayout::normal_string);
	asserts(fmt_str_normal.size() == 30);


	printf("String Test Done.\n");

	// std::system("pause");

	return 0;
}