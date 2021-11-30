
rem need space after 0, 0>shared.txt doesn't work 0 means stdin, so first "echo 0 >shared.txt"
rem https://stackoverflow.com/questions/17779784/batch-file-echo-line-with-0-does-not-write-to-file
rem or just use "00>shared.txt" but when bridgeTest.exe got number 1 and print it to file so file content will be "10" so digits from 2-9 are omitted
echo 0 >shared.txt

rem clear file contents
rem https://stackoverflow.com/questions/19633676/how-to-delete-content-from-a-text-file-using-windows-batch-script/19633987
break>results.txt

FOR /L %%A IN (0,1,5) DO (
  start Release\bridgeTest.exe %%A
)

