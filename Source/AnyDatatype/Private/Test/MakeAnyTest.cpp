#include "Misc/AutomationTest.h"

#include "Any.hpp"

BEGIN_DEFINE_SPEC(MakeAnyTest, "AnyDatatype.MakeAny", EAutomationTestFlags::ProductFilter | EAutomationTestFlags::ApplicationContextMask)
END_DEFINE_SPEC(MakeAnyTest)

struct FMakeAnyTestClass {
	TArray<int> Values;
	
	FMakeAnyTestClass() {}
	FMakeAnyTestClass(int Value) : Values{ Value } {}
	FMakeAnyTestClass(std::initializer_list<int> InitList) : Values{InitList} {}
};

void MakeAnyTest::Define()
{
	It("should work with a default constructor", [this]
		{
			FAny AnyFromFloat = MakeAny<FMakeAnyTestClass>();
			TestTrue("Value set", AnyFromFloat.HasValue());
			TestTrue("Type is valid", AnyFromFloat.CanCastTo<FMakeAnyTestClass>());
			TestTrue("Value is valid", AnyCast<FMakeAnyTestClass>(AnyFromFloat).Values.Num() == 0);
		});

	It("should work with a single arg constructor", [this]
	{
		FAny AnyFromFloat = MakeAny<FMakeAnyTestClass>(42);
		TestTrue("Value set", AnyFromFloat.HasValue());
		TestTrue("Type is valid", AnyFromFloat.CanCastTo<FMakeAnyTestClass>());
		TestEqual("Value is valid", AnyCast<FMakeAnyTestClass>(AnyFromFloat).Values, {42});
	});

	It("should work with a initializer list", [this]
	{
		const std::initializer_list<int> InitValues{ 1, 2, 3 };
			
		FAny AnyFromFloat = MakeAny<FMakeAnyTestClass>(InitValues);
		TestTrue("Value set", AnyFromFloat.HasValue());
		TestTrue("Type is valid", AnyFromFloat.CanCastTo<FMakeAnyTestClass>());
		TestEqual("Value is valid", AnyCast<FMakeAnyTestClass>(AnyFromFloat).Values, TArray<int>{InitValues});
	});
}
