@echo off
git add .
set /p message= "Enter commit message: "
git commit -m "%message%"
git push