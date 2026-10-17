# SPDX-License-Identifier: GPL-2.0-or-later
$ErrorActionPreference = 'Stop'
Set-Location $PSScriptRoot
if (Get-Command py -ErrorAction SilentlyContinue) {
    & py -3 (Join-Path $PSScriptRoot 'scripts/quest-builder.py')
} elseif (Get-Command python -ErrorAction SilentlyContinue) {
    & python (Join-Path $PSScriptRoot 'scripts/quest-builder.py')
} else {
    throw 'Python 3 mit Tkinter fehlt. Bitte von https://www.python.org/downloads/ installieren.'
}
exit $LASTEXITCODE
