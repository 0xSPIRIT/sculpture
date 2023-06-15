@echo off

echo:
echo ------ Nocheckins ------
echo:
rg "nocheckin" --line-number -tc


echo:
echo ------ TODO ------
echo:
rg "TODO" --line-number -tc
