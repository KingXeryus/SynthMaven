# Clipper: publish the release on GitHub, from this folder (PUBLIER-GITHUB.bat).
#   1. the source: clipper-1.0.bundle -> the repository OWNER/REPO (created if it does not exist), branch main, the tag
#   2. the pre-release v1.0-experimental: docs/release-1.0.md, the firmware package (if INSTALL-CLIPPER.bat built it),
#      the demo WAV
#   3. the installer and the editor on GitHub Pages (branch gh-pages, from build\clipper-site)
# Uses your own Git and GitHub CLI (gh) and your own GitHub login. Nothing is ever forced on main: a repository that
# already has another history stops the script. gh-pages is generated: it is replaced each time.
param([string]$Owner = "", [string]$Repo = "clipper-fm1", [string]$Tag = "v1.0-experimental")
$ErrorActionPreference = 'Continue'
Set-Location (Split-Path $PSScriptRoot)
function Fail([string]$m) { Write-Host ""; Write-Host "ERREUR : $m" -ForegroundColor Red; exit 1 }
function Step([string]$m) { Write-Host "== $m" -ForegroundColor Cyan }

Write-Host ""
Write-Host "  C L I P P E R   1.0   EXPERIMENTAL   ->   GitHub" -ForegroundColor White
Write-Host "  ---- ---- ---- ----" -ForegroundColor DarkGray
foreach ($t in @("git", "gh")) {
  if (-not (Get-Command $t -ErrorAction SilentlyContinue)) {
    Fail "$t est introuvable. Installe Git (https://git-scm.com) et GitHub CLI (https://cli.github.com), puis relance."
  }
}
gh auth status *> $null
if ($LASTEXITCODE -ne 0) {
  Step "Connexion a GitHub (le navigateur va s'ouvrir)"
  gh auth login --web --git-protocol https
  if ($LASTEXITCODE -ne 0) { Fail "la connexion a GitHub n'a pas abouti" }
}
gh auth setup-git *> $null
if (-not $Owner) { $Owner = "$(gh api user --jq .login)".Trim() }
if (-not $Owner) { Fail "impossible de lire ton nom d'utilisateur GitHub" }
$full = "$Owner/$Repo"
$pages = "https://$Owner.github.io/$Repo"
$bundle = "clipper-1.0.bundle"
if (-not (Test-Path $bundle)) { Fail "$bundle introuvable dans ce dossier" }
$work = Join-Path "build" "publish"
$repoDir = Join-Path $work "repo"
if (Test-Path $work) { Remove-Item -Recurse -Force $work }
New-Item -ItemType Directory -Force $work | Out-Null
$email = "$Owner@users.noreply.github.com"

# 1. the source
Step "Le code (depuis $bundle)"
git clone --quiet --branch main $bundle $repoDir
if ($LASTEXITCODE -ne 0 -or -not (Test-Path $repoDir)) { Fail "git clone du bundle" }
Push-Location $repoDir
gh repo view $full *> $null
if ($LASTEXITCODE -ne 0) {
  Step "Creation du depot public $full"
  gh repo create $full --public --description "Clipper: an experimental synthesizer firmware for the M-VAVE FM-1 (from SLOOP, based on Felucca)" --homepage "$pages/webapp/installer/"
  if ($LASTEXITCODE -ne 0) { Pop-Location; Fail "la creation du depot $full" }
}
git remote remove origin *> $null
git remote add origin "https://github.com/$full.git"
Step "Envoi de la branche main vers $full"
git push -u origin main
if ($LASTEXITCODE -ne 0) {
  Pop-Location
  Fail "git push a ete refuse (le depot $full a peut-etre deja un autre historique). Rien n'a ete force. Choisis un autre nom : PUBLIER-GITHUB.bat -Repo autre-nom"
}
git -c user.name="$Owner" -c user.email="$email" tag -a $Tag -m "Clipper 1.0 (experimental)" 2> $null
git push origin $Tag
if ($LASTEXITCODE -ne 0) { Pop-Location; Fail "l'envoi du tag $Tag" }
$notes = Join-Path (Resolve-Path "..").Path "release-notes.md"
$src = Join-Path "docs" "release-1.0.md"
if (-not (Test-Path $src)) { Pop-Location; Fail "docs/release-1.0.md manque dans le bundle (un bundle trop ancien ?)" }
$text = (Get-Content -Raw -Encoding UTF8 $src).Replace("https://isod89.github.io/clipper-fm1", $pages).Replace("isod89/clipper-fm1", $full)
[IO.File]::WriteAllText($notes, $text, (New-Object Text.UTF8Encoding $false))   # (no BOM: GitHub shows the text as it is)
Pop-Location

# 2. the pre-release
$site = Join-Path "build" "clipper-site"
$assets = @()
$fw = Get-ChildItem (Join-Path (Join-Path $site "firmware") "clipper-*.fwsc") -ErrorAction SilentlyContinue | Select-Object -First 1
if ($fw) { $assets += $fw.FullName }
else { Write-Host "(Pas de firmware compile : lance d'abord INSTALL-CLIPPER.bat. La release est publiee sans le .fwsc ; relance ce script ensuite pour l'ajouter.)" -ForegroundColor Yellow }
if (Test-Path "clipper-1.0-demo.wav") { $assets += (Resolve-Path "clipper-1.0-demo.wav").Path }
gh release view $Tag --repo $full *> $null
if ($LASTEXITCODE -eq 0) {
  Step "La release $Tag existe : mise a jour de ses notes et fichiers"
  gh release edit $Tag --repo $full --prerelease --title "Clipper 1.0 (experimental)" --notes-file $notes
  if ($assets.Count -gt 0) { gh release upload $Tag @assets --repo $full --clobber }
} else {
  Step "Creation de la pre-release $Tag"
  gh release create $Tag @assets --repo $full --prerelease --title "Clipper 1.0 (experimental)" --notes-file $notes
}
if ($LASTEXITCODE -ne 0) { Fail "la release $Tag" }

# 3. GitHub Pages: the installer and the editor
if (Test-Path (Join-Path (Join-Path (Join-Path $site "webapp") "installer") "index.html")) {
  Step "L'installateur et l'editeur sur GitHub Pages"
  $pg = Join-Path $work "pages"
  New-Item -ItemType Directory -Force $pg | Out-Null
  Copy-Item -Recurse (Join-Path $site "*") $pg
  New-Item -ItemType File (Join-Path $pg ".nojekyll") | Out-Null
  Push-Location $pg
  git init --quiet
  git checkout --quiet -b gh-pages
  git add -A
  git -c user.name="$Owner" -c user.email="$email" commit --quiet -m "Clipper 1.0 (experimental): the installer and the editor"
  git push --force "https://github.com/$full.git" gh-pages
  $ok = $LASTEXITCODE -eq 0
  Pop-Location
  if (-not $ok) { Fail "l'envoi de la branche gh-pages" }
  gh api -X POST "repos/$full/pages" -f "source[branch]=gh-pages" -f "source[path]=/" *> $null   # (already on: refused, fine)
  Write-Host "   (GitHub met une a deux minutes a publier la page la premiere fois)" -ForegroundColor DarkGray
} else {
  Write-Host "(Pas de build\clipper-site : lance d'abord INSTALL-CLIPPER.bat pour publier l'installateur sur GitHub Pages.)" -ForegroundColor Yellow
}

Write-Host ""
Write-Host "Depot :         https://github.com/$full" -ForegroundColor Green
Write-Host "Release :       https://github.com/$full/releases/tag/$Tag" -ForegroundColor Green
Write-Host "Installateur :  $pages/webapp/installer/" -ForegroundColor Green
Write-Host "Editeur :       $pages/webapp/editor/" -ForegroundColor Green
