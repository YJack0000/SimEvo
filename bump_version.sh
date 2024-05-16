#!/bin/bash

# Get the current version from setup.py
VERSION=$(python setup.py --version)
echo "Current version: $VERSION"

# Increment the version number
NEW_VERSION=$(echo $VERSION | awk -F. -v OFS=. '{$NF++; print}')
echo "New version: $NEW_VERSION"

# Update setup.py with the new version
echo "Updating setup.py from version='$VERSION' to version='$NEW_VERSION'"
sed -i '' "s/version='$VERSION'/version='$NEW_VERSION'/" setup.py

# Ensure the version was updated
grep "version='$NEW_VERSION'" setup.py
if [ $? -eq 0 ]; then
    echo "Version updated successfully"
else
    echo "Failed to update version"
    exit 1  # Exit with an error
fi

# Output the new version
echo "::set-output name=version::$NEW_VERSION"

