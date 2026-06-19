# Releasing LogView

This document describes how to publish a new release of LogView. Releases are
built and published automatically by GitHub Actions whenever a version tag
(`v*`) is pushed.

## Overview

- The single source of truth for the version is `set(version ...)` in
  `CMakeLists.txt`. It is compiled into the binary as `APP_VERSION` and is what
  the in-app **Help → Check for updates...** feature compares against the latest
  GitHub release tag.
- The release workflow (`.github/workflows/release.yml`) triggers on tags
  matching `v*`. It builds Linux and Windows binaries and attaches them to a new
  GitHub Release.
- For the in-app updater to detect a release, the Git tag must match the version
  baked into the binary, e.g. binary version `0.0.5` ↔ tag `v0.0.5`.

## Release steps

### 1. Bump the version

Edit `CMakeLists.txt` and update the version line:

```cmake
set(version \"0.0.5\")
```

Use [semantic versioning](https://semver.org/): `MAJOR.MINOR.PATCH`.

### 2. Commit the version bump

```bash
git add CMakeLists.txt
git commit -m "Bump version to 0.0.5"
git push origin master
```

### 3. Create and push the tag

The tag **must** be the version prefixed with `v`:

```bash
git tag v0.0.5
git push origin v0.0.5
```

Pushing the tag is what triggers the release workflow.

### 4. Wait for the workflow

Watch the run under the repository's **Actions → Release** tab. It runs two
jobs:

1. **build-linux** — builds on Ubuntu, packages `LogView-linux-x64.tar.gz`, and
   creates the GitHub Release with auto-generated release notes.
2. **build-windows** — runs after the Linux job, builds with Qt on Windows,
   bundles the Qt runtime with `windeployqt`, and attaches
   `LogView-windows-x64.zip` to the same release.

When both jobs finish, the release at
`https://github.com/radx64/LogView/releases` will contain both assets.

### 5. (Optional) Polish the release notes

The workflow auto-generates release notes from merged pull requests and commits.
You can edit the release on GitHub to add a summary or highlights. **This text
is exactly what users see in the in-app update dialog**, so it is worth making it
readable. The notes are rendered as Markdown in the app.

## How the in-app update check works

- **Help → Check for updates...** queries
  `https://api.github.com/repos/radx64/LogView/releases/latest` and compares the
  release `tag_name` against the built-in `APP_VERSION`.
- If a newer version exists, a dialog shows the release notes and a **Download**
  link to the release page, plus a **Skip this version** option.
- An automatic check also runs shortly after startup (configurable under
  **Options → Updates**). The automatic check stays silent unless a new,
  non-skipped version is available.
- Only fully **published** releases are detected. Drafts and pre-releases are
  ignored by the `/releases/latest` endpoint.

## Notes and gotchas

- **Tag must match the binary version.** If the tag (`v0.0.5`) is newer than the
  version compiled into a user's installed binary, they will be prompted to
  update. If you forget to bump `CMakeLists.txt` before tagging, the published
  binaries will report an older version than their tag.
- **Re-running a release.** If you need to re-trigger the workflow for an
  existing tag, delete the tag locally and remotely, then recreate and push it:
  ```bash
  git push --delete origin v0.0.5
  git tag --delete v0.0.5
  git tag v0.0.5
  git push origin v0.0.5
  ```
  (You may also want to delete the existing GitHub Release first.)
- **Qt version.** The Windows job pins Qt (see `version:` in
  `.github/workflows/release.yml`). Bump it there if you move to a newer Qt.
- **GitHub API rate limits.** The in-app check is unauthenticated and limited to
  ~60 requests/hour per IP — fine for manual and once-per-startup checks.
