param(
    [string]$Root = $PSScriptRoot,
    [int]$Port = 8765
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$rootPath = (Resolve-Path -LiteralPath $Root).Path.TrimEnd('\')
$prefix = "http://127.0.0.1:$Port/"

$contentTypes = @{
    '.html' = 'text/html; charset=utf-8'
    '.htm'  = 'text/html; charset=utf-8'
    '.js'   = 'text/javascript; charset=utf-8'
    '.css'  = 'text/css; charset=utf-8'
    '.json' = 'application/json; charset=utf-8'
    '.txt'  = 'text/plain; charset=utf-8'
}

function Send-Response {
    param(
        [System.Net.Sockets.NetworkStream]$Stream,
        [int]$StatusCode,
        [string]$Reason,
        [string]$ContentType,
        [byte[]]$Body
    )

    $header = "HTTP/1.1 $StatusCode $Reason`r`n" +
        "Content-Type: $ContentType`r`n" +
        "Content-Length: $($Body.Length)`r`n" +
        "Connection: close`r`n" +
        "Cache-Control: no-store`r`n" +
        "`r`n"

    $headerBytes = [System.Text.Encoding]::ASCII.GetBytes($header)
    $Stream.Write($headerBytes, 0, $headerBytes.Length)
    if ($Body.Length -gt 0) {
        $Stream.Write($Body, 0, $Body.Length)
    }
}

function Send-TextResponse {
    param(
        [System.Net.Sockets.NetworkStream]$Stream,
        [int]$StatusCode,
        [string]$Reason,
        [string]$Body
    )

    $bytes = [System.Text.Encoding]::UTF8.GetBytes($Body)
    Send-Response -Stream $Stream -StatusCode $StatusCode -Reason $Reason -ContentType 'text/plain; charset=utf-8' -Body $bytes
}

$server = [System.Net.Sockets.TcpListener]::new([System.Net.IPAddress]::Loopback, $Port)

try {
    $server.Start()
    Write-Host "Serving $rootPath"
    Write-Host "Open ${prefix}privacy-trigger-test.html"
    Write-Host "Press Ctrl+C in this window to stop the server."

    while ($true) {
        $client = $server.AcceptTcpClient()

        try {
            $stream = $client.GetStream()
            $reader = [System.IO.StreamReader]::new(
                $stream,
                [System.Text.Encoding]::ASCII,
                $false,
                1024,
                $true
            )

            $requestLine = $reader.ReadLine()
            if ([string]::IsNullOrWhiteSpace($requestLine)) {
                Send-TextResponse -Stream $stream -StatusCode 400 -Reason 'Bad Request' -Body 'Bad request'
                continue
            }

            while ($true) {
                $line = $reader.ReadLine()
                if ($null -eq $line -or $line.Length -eq 0) {
                    break
                }
            }

            $parts = $requestLine.Split(' ')
            if ($parts.Length -lt 2 -or $parts[0] -ne 'GET') {
                Send-TextResponse -Stream $stream -StatusCode 405 -Reason 'Method Not Allowed' -Body 'Only GET is supported'
                continue
            }

            $urlPath = $parts[1].Split('?')[0].TrimStart('/')
            $requestPath = [Uri]::UnescapeDataString($urlPath)
            if ([string]::IsNullOrWhiteSpace($requestPath)) {
                $requestPath = 'privacy-trigger-test.html'
            }

            $requestPath = $requestPath.Replace('/', [IO.Path]::DirectorySeparatorChar)
            $candidate = Join-Path $rootPath $requestPath

            if (-not (Test-Path -LiteralPath $candidate -PathType Leaf)) {
                Send-TextResponse -Stream $stream -StatusCode 404 -Reason 'Not Found' -Body 'Not found'
                continue
            }

            $resolved = (Resolve-Path -LiteralPath $candidate).Path
            if (-not $resolved.StartsWith($rootPath, [StringComparison]::OrdinalIgnoreCase)) {
                Send-TextResponse -Stream $stream -StatusCode 403 -Reason 'Forbidden' -Body 'Forbidden'
                continue
            }

            $extension = [IO.Path]::GetExtension($resolved).ToLowerInvariant()
            if ($contentTypes.ContainsKey($extension)) {
                $contentType = $contentTypes[$extension]
            } else {
                $contentType = 'application/octet-stream'
            }

            $bytes = [IO.File]::ReadAllBytes($resolved)
            Send-Response -Stream $stream -StatusCode 200 -Reason 'OK' -ContentType $contentType -Body $bytes
        } catch {
            try {
                Send-TextResponse -Stream $stream -StatusCode 500 -Reason 'Internal Server Error' -Body $_.Exception.Message
            } catch {
            }
        } finally {
            $client.Close()
        }
    }
} finally {
    $server.Stop()
}
