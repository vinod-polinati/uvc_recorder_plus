# Publishing `uvc_recorder_plus` to pub.dev

## Prerequisites
1. **Google Account**: You need a Google account to publish.
2. **Verified Publisher**: (Optional but recommended) Verify your domain if you want a "verified publisher" badge.

## Preparation
Before publishing, ensure your code is hosted on a public repository (like GitHub) because `pub.dev` displays the repository link.
1. Create a new repository (e.g., `uvc_recorder_plus`) on GitHub.
2. Push the contents of this folder (`plugins/uvc_manager_fork`) to that repository.
   - *Note: You can publish without this, but the 'Homepage' link in `pubspec.yaml` should point to a valid URL.*

## publishing Steps

### 1. Login
Open your terminal in this directory (`plugins/uvc_manager_fork`) and run:
```bash
flutter pub login
```
Follow the URL to authenticate with your Google account.

### 2. Verify (Dry Run)
We already ran this, but it's good practice to check one last time:
```bash
flutter pub publish --dry-run
```
This checks for warnings or errors without actually publishing.

### 3. Publish
To publish the package permanently:
```bash
flutter pub publish
```
- You will be asked to confirm. Type `y` and press Enter.
- The package will be uploaded.

### 4. Verification
- You will receive an email confirming the publication.
- Visit `https://pub.dev/packages/uvc_recorder_plus` to see your package live!

## Future Updates
1. Make changes to the code.
2. Update the `version` in `pubspec.yaml` (e.g., `1.1.0` -> `1.1.1`).
3. Update `CHANGELOG.md`.
4. Run `flutter pub publish`.
