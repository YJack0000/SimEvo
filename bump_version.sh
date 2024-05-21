#!/bin/bash

# Get the current version from setup.py
VERSION=$(python setup.py --version)
echo "Current version: $VERSION"

# Increment the version number
NEW_VERSION=$(echo $VERSION | awk -F. -v OFS=. '{$NF++; print}')
echo "New version: $NEW_VERSION"

# Update setup.py with the new version
echo "Updating setup.py from version='$VERSION' to version='$NEW_VERSION'"
if [[ "$OSTYPE" == "darwin"* ]]; then
  # macOS
  sed -i '' "s/version='$VERSION'/version='$NEW_VERSION'/" setup.py
else
  # Linux
  sed -i "s/version='$VERSION'/version='$NEW_VERSION'/" setup.py
fi

# Ensure the version was updated
if grep "version='$NEW_VERSION'" setup.py; then
    echo "Version updated successfully"
else
    echo "Failed to update version"
    exit 1  # Exit with an error
fi

# Output the new version
echo "new_version=$NEW_VERSION" >> $GITHUB_OUTPUT

