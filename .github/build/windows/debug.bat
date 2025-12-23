for %%x in (
        JAMP
		JASP
		JK2MP
		JK2SP
		MOHAA
		MOHBT
		MOHSH
		QUAKE2
		Q3A
		RTCWMP
		RTCWSP
		SIN
		SOF2MP
		STEF2
		STVOYHM
		STVOYSP
		WET
       ) do (
         msbuild .\msvc\test_qmm.vcxproj /p:Configuration=Debug-%%x /p:Platform=x86
         msbuild .\msvc\test_qmm.vcxproj /p:Configuration=Debug-%%x /p:Platform=x64
       )

msbuild .\msvc\test_qmm.vcxproj /p:Configuration=Debug-Q2R /p:Platform=x64
