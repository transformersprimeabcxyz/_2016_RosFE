<module name="bltrop" type="win32gui" installbase="bin" installname="bltrop.exe">
	<define name="__USE_W32API" />
	<include base="bltrop">.</include>
	<library>kernel32</library>
	<library>gdi32</library>
	<library>user32</library>
	<file>bltrop.c</file>
	<file>bltrop.rc</file>
</module>