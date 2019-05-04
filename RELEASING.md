# Guideline to publishing new release

## MSIRGB
 1. Update assembly info for binaries with new version number
 2. Package clean release build into 7z archive (excluding ./Scripts/ folder)
 3. Publish on GitHub Releases with tag 'vx.x.x'
 4. Update README's "How to install" section to point to new release

## Scripts for MSIRGB
 1. Package ./Scripts/ folder contents into 7z archive
 2. Publish on GitHub Releases with tag 'scripts-vx.x.x'
 3. Update README's "How to install" section to point to new release
