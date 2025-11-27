
# Using this package

This package contains SDL3_image built for the Android platform.

## Gradle integration

For integration with CMake/ndk-build, it uses [prefab](https://google.github.io/prefab/).

Copy the aar archive (SDL3_image-3.2.4.aar) to a `app/libs` directory of your project.

In `app/build.gradle` of your Android project, add:
```
android {
    /* ... */
    buildFeatures {
        prefab true
    }
}
dependencies {
    implementation files('libs/SDL3_image-3.2.4.aar')
    /* ... */
}
```

If you're using CMake, add the following to your CMakeLists.txt:
```
find_package(SDL3_image REQUIRED CONFIG)
target_link_libraries(yourgame PRIVATE SDL3_image::SDL3_image)
```

If you use ndk-build, add the following before `include $(BUILD_SHARED_LIBRARY)` to your `Android.mk`:
```
LOCAL_SHARED_LIBARARIES := SDL3_image
```
And add the following at the bottom:
```
# https://google.github.io/prefab/build-systems.html

# Add the prefab modules to the import path.
$(call import-add-path,/out)

# Import SDL3_image so we can depend on it.
$(call import-module,prefab/SDL3_image)
```

---

## Other build systems (advanced)

If you want to build a project without Gradle,
running the following command will extract the Android archive into a more common directory structure.
```
python SDL3_image-3.2.4.aar -o android_prefix
```
Add `--help` for a list of all available options.

# Documentation

An API reference and additional documentation is available at:

https://wiki.libsdl.org/SDL3_image

# Discussions

## Discord

You can join the official Discord server at:

https://discord.com/invite/BwpFGBWsv8

## Forums/mailing lists

You can join SDL development discussions at:

https://discourse.libsdl.org/

Once you sign up, you can use the forum through the website or as a mailing list from your email client.

## Announcement list

You can sign up for the low traffic announcement list at:

https://www.libsdl.org/mailing-list.php

