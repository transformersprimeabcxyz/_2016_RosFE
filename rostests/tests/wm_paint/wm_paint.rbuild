<module name="wm_paint" type="win32gui" installbase="bin" installname="wm_paint.exe">
	<define name="__USE_W32API" />
	<library>kernel32</library>
	<library>user32</library>
	<library>gdi32</library>
	<file>wm_paint.c</file>
</module>
