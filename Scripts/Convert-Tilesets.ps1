param (
  [Parameter(Mandatory=$True,Position=1)]
  [string]$jazzDir
)

Get-ChildItem -path $jazzDir -filter "*.j2t" | Foreach-Object {
    echo '`n' | ./pctc.exe $_.BaseName $_.FullName
}