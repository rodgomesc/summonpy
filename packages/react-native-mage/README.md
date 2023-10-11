# react-native-pyjsi

jsi bindings for python

## Installation

```sh
npm install react-native-pyjsi
```

## Usage

```js
import { multiply } from 'react-native-pyjsi';

// ...

const result = await multiply(3, 7);
```

## Contributing

See the [contributing guide](CONTRIBUTING.md) to learn how to contribute to the repository and the development workflow.

## License

MIT

---

Made with [create-react-native-library](https://github.com/callstack/react-native-builder-bob)


# Android Setup
To ensure that native libraries are extracted in installation time, add the following line in your AndroidManifest.xml:

  ```xml
  <application
    ...
    android:extractNativeLibs="true">
    ...
  </application>
```


# create local mamba env

paste this environment.yml

```yml
# Name
name: pyjsi-env

# Channels to look for any specific libraries
channels:
  - local

# App dependencies to be installed
dependencies:
  - python=3.10
  - pip
  - android-python * py310*
  - android-pybind
```
then create the env with envi
```sh
mamba env create -f environment.yml
```


## ARCHITECTURE FLOW

- create python extension
- inject python extension to hook custom import locations
