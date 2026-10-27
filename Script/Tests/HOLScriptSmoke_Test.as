// Smoke test of the script tests (D484). quick_check runs the script tests with the C++ ones and needs at least one
// script test, so a script that no longer compiles cannot hide behind green C++ tests.

// The script sees the project's C++ classes.
void Test_SeesProjectClasses(FUnitTest& T)
{
	T.AssertNotNull(AHOLBuilding, "AHOLBuilding bound to script");
}
