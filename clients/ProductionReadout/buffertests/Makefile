OBJECTS=TestFixture.o OutputBuffer.o ControlBuffer.o \
	BeginBuffer.o EndBuffer.o PauseBuffer.o ResumeBuffer.o \
	ScalerBuffer.o PhysicsBuffers.o StringListBuffer.o \
	DocumentationBuffer.o RunVariableBuffer.o StateVariableBuffer.o 

SPECCXX=$(shell /usr/opt/spectrodaq/bin/spectrodaq-conf --cflags)
SPECLD=$(shell /usr/opt/spectrodaq/bin/spectrodaq-conf  --libs)

CXX=c++
CXXFLAGS= -c -ggdb  -I. -I/usr/include/tcl \
	-I/usr/opt/daq/include $(SPECCXX)

.cpp.o:
	$(CXX) $(CXXFLAGS) $<


tests: BufferTests 

depend:
	makedepend *.cpp

BufferTests: $(OBJECTS)
	g++ -o BufferTests  -L/usr/opt/daq/lib \
	$(OBJECTS) -lPReadout -ltclPlus -lLicense $(SPECLD) \
	-lcppunit -lVmeAPI -lbtp -lpthread -lpub



clean:
	rm *.o


# DO NOT DELETE

BeginBuffer.o: /usr/include/cppunit/extensions/HelperMacros.h
BeginBuffer.o: /usr/include/cppunit/Portability.h
BeginBuffer.o: /usr/include/cppunit/config-auto.h
BeginBuffer.o: /usr/include/cppunit/extensions/AutoRegisterSuite.h
BeginBuffer.o: /usr/include/cppunit/extensions/TestSuiteFactory.h
BeginBuffer.o: /usr/include/cppunit/extensions/TestFactory.h
BeginBuffer.o: /usr/include/cppunit/extensions/TestFactoryRegistry.h
BeginBuffer.o: /usr/include/cppunit/extensions/TestSuiteBuilder.h
BeginBuffer.o: /usr/include/cppunit/TestSuite.h /usr/include/cppunit/Test.h
BeginBuffer.o: /usr/include/cppunit/TestCaller.h
BeginBuffer.o: /usr/include/cppunit/Exception.h
BeginBuffer.o: /usr/include/cppunit/SourceLine.h
BeginBuffer.o: /usr/lib/gcc-lib/i386-linux/2.95.4/include/exception
BeginBuffer.o: /usr/include/cppunit/TestCase.h
BeginBuffer.o: /usr/include/cppunit/TestAssert.h
BeginBuffer.o: /usr/include/cppunit/Asserter.h
BeginBuffer.o: /usr/include/cppunit/TestFixture.h /usr/include/string.h
BeginBuffer.o: /usr/include/features.h /usr/include/sys/cdefs.h
BeginBuffer.o: /usr/include/gnu/stubs.h
BeginBuffer.o: /usr/lib/gcc-lib/i386-linux/2.95.4/include/stddef.h
BeginBuffer.o: /usr/include/time.h /usr/include/bits/time.h
BeginBuffer.o: /usr/include/bits/types.h
ControlBuffer.o: /usr/include/cppunit/extensions/HelperMacros.h
ControlBuffer.o: /usr/include/cppunit/Portability.h
ControlBuffer.o: /usr/include/cppunit/config-auto.h
ControlBuffer.o: /usr/include/cppunit/extensions/AutoRegisterSuite.h
ControlBuffer.o: /usr/include/cppunit/extensions/TestSuiteFactory.h
ControlBuffer.o: /usr/include/cppunit/extensions/TestFactory.h
ControlBuffer.o: /usr/include/cppunit/extensions/TestFactoryRegistry.h
ControlBuffer.o: /usr/include/cppunit/extensions/TestSuiteBuilder.h
ControlBuffer.o: /usr/include/cppunit/TestSuite.h /usr/include/cppunit/Test.h
ControlBuffer.o: /usr/include/cppunit/TestCaller.h
ControlBuffer.o: /usr/include/cppunit/Exception.h
ControlBuffer.o: /usr/include/cppunit/SourceLine.h
ControlBuffer.o: /usr/lib/gcc-lib/i386-linux/2.95.4/include/exception
ControlBuffer.o: /usr/include/cppunit/TestCase.h
ControlBuffer.o: /usr/include/cppunit/TestAssert.h
ControlBuffer.o: /usr/include/cppunit/Asserter.h
ControlBuffer.o: /usr/include/cppunit/TestFixture.h /usr/include/string.h
ControlBuffer.o: /usr/include/features.h /usr/include/sys/cdefs.h
ControlBuffer.o: /usr/include/gnu/stubs.h
ControlBuffer.o: /usr/lib/gcc-lib/i386-linux/2.95.4/include/stddef.h
ControlBuffer.o: /usr/include/time.h /usr/include/bits/time.h
ControlBuffer.o: /usr/include/bits/types.h
EndBuffer.o: /usr/include/cppunit/extensions/HelperMacros.h
EndBuffer.o: /usr/include/cppunit/Portability.h
EndBuffer.o: /usr/include/cppunit/config-auto.h
EndBuffer.o: /usr/include/cppunit/extensions/AutoRegisterSuite.h
EndBuffer.o: /usr/include/cppunit/extensions/TestSuiteFactory.h
EndBuffer.o: /usr/include/cppunit/extensions/TestFactory.h
EndBuffer.o: /usr/include/cppunit/extensions/TestFactoryRegistry.h
EndBuffer.o: /usr/include/cppunit/extensions/TestSuiteBuilder.h
EndBuffer.o: /usr/include/cppunit/TestSuite.h /usr/include/cppunit/Test.h
EndBuffer.o: /usr/include/cppunit/TestCaller.h
EndBuffer.o: /usr/include/cppunit/Exception.h
EndBuffer.o: /usr/include/cppunit/SourceLine.h
EndBuffer.o: /usr/lib/gcc-lib/i386-linux/2.95.4/include/exception
EndBuffer.o: /usr/include/cppunit/TestCase.h
EndBuffer.o: /usr/include/cppunit/TestAssert.h
EndBuffer.o: /usr/include/cppunit/Asserter.h
EndBuffer.o: /usr/include/cppunit/TestFixture.h /usr/include/string.h
EndBuffer.o: /usr/include/features.h /usr/include/sys/cdefs.h
EndBuffer.o: /usr/include/gnu/stubs.h
EndBuffer.o: /usr/lib/gcc-lib/i386-linux/2.95.4/include/stddef.h
EndBuffer.o: /usr/include/time.h /usr/include/bits/time.h
EndBuffer.o: /usr/include/bits/types.h
OutputBuffer.o: /usr/include/cppunit/extensions/HelperMacros.h
OutputBuffer.o: /usr/include/cppunit/Portability.h
OutputBuffer.o: /usr/include/cppunit/config-auto.h
OutputBuffer.o: /usr/include/cppunit/extensions/AutoRegisterSuite.h
OutputBuffer.o: /usr/include/cppunit/extensions/TestSuiteFactory.h
OutputBuffer.o: /usr/include/cppunit/extensions/TestFactory.h
OutputBuffer.o: /usr/include/cppunit/extensions/TestFactoryRegistry.h
OutputBuffer.o: /usr/include/cppunit/extensions/TestSuiteBuilder.h
OutputBuffer.o: /usr/include/cppunit/TestSuite.h /usr/include/cppunit/Test.h
OutputBuffer.o: /usr/include/cppunit/TestCaller.h
OutputBuffer.o: /usr/include/cppunit/Exception.h
OutputBuffer.o: /usr/include/cppunit/SourceLine.h
OutputBuffer.o: /usr/lib/gcc-lib/i386-linux/2.95.4/include/exception
OutputBuffer.o: /usr/include/cppunit/TestCase.h
OutputBuffer.o: /usr/include/cppunit/TestAssert.h
OutputBuffer.o: /usr/include/cppunit/Asserter.h
OutputBuffer.o: /usr/include/cppunit/TestFixture.h /usr/include/string.h
OutputBuffer.o: /usr/include/features.h /usr/include/sys/cdefs.h
OutputBuffer.o: /usr/include/gnu/stubs.h
OutputBuffer.o: /usr/lib/gcc-lib/i386-linux/2.95.4/include/stddef.h
PauseBuffer.o: /usr/include/cppunit/extensions/HelperMacros.h
PauseBuffer.o: /usr/include/cppunit/Portability.h
PauseBuffer.o: /usr/include/cppunit/config-auto.h
PauseBuffer.o: /usr/include/cppunit/extensions/AutoRegisterSuite.h
PauseBuffer.o: /usr/include/cppunit/extensions/TestSuiteFactory.h
PauseBuffer.o: /usr/include/cppunit/extensions/TestFactory.h
PauseBuffer.o: /usr/include/cppunit/extensions/TestFactoryRegistry.h
PauseBuffer.o: /usr/include/cppunit/extensions/TestSuiteBuilder.h
PauseBuffer.o: /usr/include/cppunit/TestSuite.h /usr/include/cppunit/Test.h
PauseBuffer.o: /usr/include/cppunit/TestCaller.h
PauseBuffer.o: /usr/include/cppunit/Exception.h
PauseBuffer.o: /usr/include/cppunit/SourceLine.h
PauseBuffer.o: /usr/lib/gcc-lib/i386-linux/2.95.4/include/exception
PauseBuffer.o: /usr/include/cppunit/TestCase.h
PauseBuffer.o: /usr/include/cppunit/TestAssert.h
PauseBuffer.o: /usr/include/cppunit/Asserter.h
PauseBuffer.o: /usr/include/cppunit/TestFixture.h /usr/include/string.h
PauseBuffer.o: /usr/include/features.h /usr/include/sys/cdefs.h
PauseBuffer.o: /usr/include/gnu/stubs.h
PauseBuffer.o: /usr/lib/gcc-lib/i386-linux/2.95.4/include/stddef.h
PauseBuffer.o: /usr/include/time.h /usr/include/bits/time.h
PauseBuffer.o: /usr/include/bits/types.h
PhysicsBuffers.o: /usr/include/cppunit/extensions/HelperMacros.h
PhysicsBuffers.o: /usr/include/cppunit/Portability.h
PhysicsBuffers.o: /usr/include/cppunit/config-auto.h
PhysicsBuffers.o: /usr/include/cppunit/extensions/AutoRegisterSuite.h
PhysicsBuffers.o: /usr/include/cppunit/extensions/TestSuiteFactory.h
PhysicsBuffers.o: /usr/include/cppunit/extensions/TestFactory.h
PhysicsBuffers.o: /usr/include/cppunit/extensions/TestFactoryRegistry.h
PhysicsBuffers.o: /usr/include/cppunit/extensions/TestSuiteBuilder.h
PhysicsBuffers.o: /usr/include/cppunit/TestSuite.h
PhysicsBuffers.o: /usr/include/cppunit/Test.h
PhysicsBuffers.o: /usr/include/cppunit/TestCaller.h
PhysicsBuffers.o: /usr/include/cppunit/Exception.h
PhysicsBuffers.o: /usr/include/cppunit/SourceLine.h
PhysicsBuffers.o: /usr/lib/gcc-lib/i386-linux/2.95.4/include/exception
PhysicsBuffers.o: /usr/include/cppunit/TestCase.h
PhysicsBuffers.o: /usr/include/cppunit/TestAssert.h
PhysicsBuffers.o: /usr/include/cppunit/Asserter.h
PhysicsBuffers.o: /usr/include/cppunit/TestFixture.h /usr/include/string.h
PhysicsBuffers.o: /usr/include/features.h /usr/include/sys/cdefs.h
PhysicsBuffers.o: /usr/include/gnu/stubs.h
PhysicsBuffers.o: /usr/lib/gcc-lib/i386-linux/2.95.4/include/stddef.h
PhysicsBuffers.o: /usr/include/time.h /usr/include/bits/time.h
PhysicsBuffers.o: /usr/include/bits/types.h
ResumeBuffer.o: /usr/include/cppunit/extensions/HelperMacros.h
ResumeBuffer.o: /usr/include/cppunit/Portability.h
ResumeBuffer.o: /usr/include/cppunit/config-auto.h
ResumeBuffer.o: /usr/include/cppunit/extensions/AutoRegisterSuite.h
ResumeBuffer.o: /usr/include/cppunit/extensions/TestSuiteFactory.h
ResumeBuffer.o: /usr/include/cppunit/extensions/TestFactory.h
ResumeBuffer.o: /usr/include/cppunit/extensions/TestFactoryRegistry.h
ResumeBuffer.o: /usr/include/cppunit/extensions/TestSuiteBuilder.h
ResumeBuffer.o: /usr/include/cppunit/TestSuite.h /usr/include/cppunit/Test.h
ResumeBuffer.o: /usr/include/cppunit/TestCaller.h
ResumeBuffer.o: /usr/include/cppunit/Exception.h
ResumeBuffer.o: /usr/include/cppunit/SourceLine.h
ResumeBuffer.o: /usr/lib/gcc-lib/i386-linux/2.95.4/include/exception
ResumeBuffer.o: /usr/include/cppunit/TestCase.h
ResumeBuffer.o: /usr/include/cppunit/TestAssert.h
ResumeBuffer.o: /usr/include/cppunit/Asserter.h
ResumeBuffer.o: /usr/include/cppunit/TestFixture.h /usr/include/string.h
ResumeBuffer.o: /usr/include/features.h /usr/include/sys/cdefs.h
ResumeBuffer.o: /usr/include/gnu/stubs.h
ResumeBuffer.o: /usr/lib/gcc-lib/i386-linux/2.95.4/include/stddef.h
ResumeBuffer.o: /usr/include/time.h /usr/include/bits/time.h
ResumeBuffer.o: /usr/include/bits/types.h
Sample.o: /usr/include/cppunit/extensions/HelperMacros.h
Sample.o: /usr/include/cppunit/Portability.h
Sample.o: /usr/include/cppunit/config-auto.h
Sample.o: /usr/include/cppunit/extensions/AutoRegisterSuite.h
Sample.o: /usr/include/cppunit/extensions/TestSuiteFactory.h
Sample.o: /usr/include/cppunit/extensions/TestFactory.h
Sample.o: /usr/include/cppunit/extensions/TestFactoryRegistry.h
Sample.o: /usr/include/cppunit/extensions/TestSuiteBuilder.h
Sample.o: /usr/include/cppunit/TestSuite.h /usr/include/cppunit/Test.h
Sample.o: /usr/include/cppunit/TestCaller.h /usr/include/cppunit/Exception.h
Sample.o: /usr/include/cppunit/SourceLine.h
Sample.o: /usr/lib/gcc-lib/i386-linux/2.95.4/include/exception
Sample.o: /usr/include/cppunit/TestCase.h /usr/include/cppunit/TestAssert.h
Sample.o: /usr/include/cppunit/Asserter.h /usr/include/cppunit/TestFixture.h
Sample.o: /usr/include/stdio.h /usr/include/features.h
Sample.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
Sample.o: /usr/lib/gcc-lib/i386-linux/2.95.4/include/stddef.h
Sample.o: /usr/include/bits/types.h /usr/include/libio.h
Sample.o: /usr/include/_G_config.h /usr/include/wchar.h
Sample.o: /usr/include/bits/wchar.h /usr/include/gconv.h
Sample.o: /usr/lib/gcc-lib/i386-linux/2.95.4/include/stdarg.h
Sample.o: /usr/include/bits/stdio_lim.h
ScalerBuffer.o: /usr/include/cppunit/extensions/HelperMacros.h
ScalerBuffer.o: /usr/include/cppunit/Portability.h
ScalerBuffer.o: /usr/include/cppunit/config-auto.h
ScalerBuffer.o: /usr/include/cppunit/extensions/AutoRegisterSuite.h
ScalerBuffer.o: /usr/include/cppunit/extensions/TestSuiteFactory.h
ScalerBuffer.o: /usr/include/cppunit/extensions/TestFactory.h
ScalerBuffer.o: /usr/include/cppunit/extensions/TestFactoryRegistry.h
ScalerBuffer.o: /usr/include/cppunit/extensions/TestSuiteBuilder.h
ScalerBuffer.o: /usr/include/cppunit/TestSuite.h /usr/include/cppunit/Test.h
ScalerBuffer.o: /usr/include/cppunit/TestCaller.h
ScalerBuffer.o: /usr/include/cppunit/Exception.h
ScalerBuffer.o: /usr/include/cppunit/SourceLine.h
ScalerBuffer.o: /usr/lib/gcc-lib/i386-linux/2.95.4/include/exception
ScalerBuffer.o: /usr/include/cppunit/TestCase.h
ScalerBuffer.o: /usr/include/cppunit/TestAssert.h
ScalerBuffer.o: /usr/include/cppunit/Asserter.h
ScalerBuffer.o: /usr/include/cppunit/TestFixture.h /usr/include/string.h
ScalerBuffer.o: /usr/include/features.h /usr/include/sys/cdefs.h
ScalerBuffer.o: /usr/include/gnu/stubs.h
ScalerBuffer.o: /usr/lib/gcc-lib/i386-linux/2.95.4/include/stddef.h
ScalerBuffer.o: /usr/include/time.h /usr/include/bits/time.h
ScalerBuffer.o: /usr/include/bits/types.h /usr/include/stdlib.h
StringListBuffer.o: /usr/include/cppunit/extensions/HelperMacros.h
StringListBuffer.o: /usr/include/cppunit/Portability.h
StringListBuffer.o: /usr/include/cppunit/config-auto.h
StringListBuffer.o: /usr/include/cppunit/extensions/AutoRegisterSuite.h
StringListBuffer.o: /usr/include/cppunit/extensions/TestSuiteFactory.h
StringListBuffer.o: /usr/include/cppunit/extensions/TestFactory.h
StringListBuffer.o: /usr/include/cppunit/extensions/TestFactoryRegistry.h
StringListBuffer.o: /usr/include/cppunit/extensions/TestSuiteBuilder.h
StringListBuffer.o: /usr/include/cppunit/TestSuite.h
StringListBuffer.o: /usr/include/cppunit/Test.h
StringListBuffer.o: /usr/include/cppunit/TestCaller.h
StringListBuffer.o: /usr/include/cppunit/Exception.h
StringListBuffer.o: /usr/include/cppunit/SourceLine.h
StringListBuffer.o: /usr/lib/gcc-lib/i386-linux/2.95.4/include/exception
StringListBuffer.o: /usr/include/cppunit/TestCase.h
StringListBuffer.o: /usr/include/cppunit/TestAssert.h
StringListBuffer.o: /usr/include/cppunit/Asserter.h
StringListBuffer.o: /usr/include/cppunit/TestFixture.h /usr/include/string.h
StringListBuffer.o: /usr/include/features.h /usr/include/sys/cdefs.h
StringListBuffer.o: /usr/include/gnu/stubs.h
StringListBuffer.o: /usr/lib/gcc-lib/i386-linux/2.95.4/include/stddef.h
StringListBuffer.o: /usr/include/time.h /usr/include/bits/time.h
StringListBuffer.o: /usr/include/bits/types.h
TestFixture.o: /usr/include/cppunit/extensions/TestFactoryRegistry.h
TestFixture.o: /usr/include/cppunit/Portability.h
TestFixture.o: /usr/include/cppunit/config-auto.h
TestFixture.o: /usr/include/cppunit/extensions/TestFactory.h
TestFixture.o: /usr/include/cppunit/ui/text/TestRunner.h
