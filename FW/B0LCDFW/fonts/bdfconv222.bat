cls
set "bdfconv_2_22=%~d0%~p0bdfconv\bdfconv_2_22.exe"
rem
set font_name=u8g2_font_7x13B




rem
del "%~d0%~p0%font_name%.h"

rem 46-102
rem %bdfconv_2_22% -v -f 1 -m "46-58" %font_bdf% -o %file_c%  -n %font_name%
rem u8g_font_04b_03b 46-86 
rem u8g2_font_7x13B num % 25-39 37-57
%bdfconv_2_22% -v -f 1 -m "37-57" "%~d0%~p0%font_name%.bdf" -o "%~d0%~p0%font_name%.h"  -n %font_name%


rem pause




rem example
rem bdfconv_2_22.exe -v -f 1 -m "32-127" ../bdf/helvB18.bdf -o helvb18_tf.c  -n u8g2_font_helvB18_tf -d ../bdf/helvB18.bdf
rem type helvb18_tf.c


