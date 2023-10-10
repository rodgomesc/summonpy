import { NativeModules, Platform } from 'react-native';

const LINKING_ERROR =
  `The package 'react-native-pyjsi' doesn't seem to be linked. Make sure: \n\n` +
  Platform.select({ ios: "- You have run 'pod install'\n", default: '' }) +
  '- You rebuilt the app after installing the package\n' +
  '- You are not using Expo Go\n';

const Pyjsi = NativeModules.Pyjsi
  ? NativeModules.Pyjsi
  : new Proxy(
      {},
      {
        get() {
          throw new Error(LINKING_ERROR);
        },
      }
    );

export function multiply(a: number, b: number): Promise<number> {
  return Pyjsi.multiply(a, b);
}

import { useEffect } from 'react';

// Placeholder function to run Python code.
// Replace this with your real Python execution logic.
const runPythonCode = async (pythonCode, jsValue) => {
  console.log(`Running Python code with JS value: ${jsValue}`);
  // Execute your Python code here and return the result.
  return `Result of Python code: ${pythonCode}`;
};

const usePyEffect = (pythonCallback, dependencies) => {
  useEffect(() => {
    const pythonCode = pythonCallback(dependencies);

    runPythonCode(pythonCode, dependencies)
      .then((result) => {
        console.log(`Python code executed, result: ${result}`);
      })
      .catch((error) => {
        console.error(`Failed to execute Python code: ${error}`);
      });
  }, dependencies);
};

export default usePyEffect;
