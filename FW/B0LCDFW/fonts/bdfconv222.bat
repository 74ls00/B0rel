cls
set "bdfconv_2_22=%~d0%~p0bdfconv\bdfconv_2_22.exe"
rem

rem 46-102
rem %bdfconv_2_22% -v -f 1 -m "46-58" %font_bdf% -o %file_c%  -n %font_name%

rem u8g2_font_7x13B num % 25-39 37-57
rem %bdfconv_2_22% -v -f 1 -m "37-57" "%~d0%~p0%font_name%.bdf" -o "%~d0%~p0%font_name%.h"  -n %font_name%


rem # u8g_font_04b_03b 46-86  30-39 2e 56 / 48-57 46 86
set font_name=u8g_font_04b_03b
%bdfconv_2_22% -v -f 1 -m "46,48-57,86" "%~d0%~p0%font_name%.bdf" -o "%~d0%~p0%font_name%.h"  -n %font_name%


rem u8g2_font_ncenB18m 2d-39 45-57
rem %bdfconv_2_22% -v -f 1 -m "45-57" "%~d0%~p0%font_name%.bdf" -o "%~d0%~p0%font_name%.h"  -n %font_name%











rem example
rem bdfconv_2_22.exe -v -f 1 -m "32-127" ../bdf/helvB18.bdf -o helvb18_tf.c  -n u8g2_font_helvB18_tf -d ../bdf/helvB18.bdf
rem type helvb18_tf.c


