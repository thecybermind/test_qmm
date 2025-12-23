mkdir package
pushd package
del /q *
rem copy ..\README.md .\
rem copy ..\LICENSE .\

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
         copy ..\bin\Release-%%x\x86\test_qmm_%%x.dll .\
         copy ..\bin\Release-%%x\x64\test_qmm_x86_64_%%x.dll .\         
       )
copy ..\bin\Release-Q2R\x64\test_qmm_x86_64_Q2R.dll .\         
popd
