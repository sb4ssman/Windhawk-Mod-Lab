Write-Host "=== Task Manager Tail Stress Test ===" -ForegroundColor Cyan
Write-Host "Opening 10 Notepad instances..."

for ($i=1; $i -le 10; $i++) {
    Start-Process notepad
    Start-Sleep -Milliseconds 200
}

Write-Host "Wait 5 seconds..."
Start-Sleep -Seconds 5

Write-Host "Closing Notepads..."
Get-Process notepad | Stop-Process

Write-Host "Test Complete. Check if Task Manager stayed at the end." -ForegroundColor Green