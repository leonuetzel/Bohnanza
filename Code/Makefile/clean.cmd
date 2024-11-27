@echo off

for %%f in (*.exe) do del %%f
for %%f in (*.a) do del %%f
cd Code/Obj
rd /s /q . 2>nul