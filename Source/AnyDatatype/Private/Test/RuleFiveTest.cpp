#include "Any.hpp"

#include "Misc/AutomationTest.h"

BEGIN_DEFINE_SPEC(RuleFiveTest, "AnyDatatype.Rule of five", EAutomationTestFlags::ProductFilter | EAutomationTestFlags::ApplicationContextMask)
END_DEFINE_SPEC(RuleFiveTest)

struct FRuleFiveTestClass
{
	static bool Alive;
	
	FRuleFiveTestClass()
	{
		if (Alive)
			UE_LOG(LogTemp, Fatal, TEXT("Constructing a new FRuleFiveTestClass while another one is alive"))
			
		Alive = true;
	}
	~FRuleFiveTestClass() { Alive = false; }
};

bool FRuleFiveTestClass::Alive = false;

#define ASSERT_CLASS_WAS_CONSTRUCTED TestTrue("Object constructor called", FRuleFiveTestClass::Alive);
#define ASSERT_CLASS_WAS_DESTRUCTED TestFalse("Object destructor called", FRuleFiveTestClass::Alive);

void RuleFiveTest::Define()
{
	Describe("Destructor and Reset", [this]
	{
		It("Reset should remove stored value", [this]
		{
			FAny Any = MakeAny<float>();
			Any.Reset();
			TestFalse("Value is not set", Any.HasValue());
			ASSERT_CLASS_WAS_DESTRUCTED;
		});
		
		It("Destructor should destruct stored object", [this]
		{
			{
				FAny Any = MakeAny<FRuleFiveTestClass>();
				ASSERT_CLASS_WAS_CONSTRUCTED;
			}
			ASSERT_CLASS_WAS_DESTRUCTED;
		});
	});
}
