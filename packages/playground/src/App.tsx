import * as React from "react";

import { StyleSheet, View } from "react-native";
import { NativeModules } from "react-native";
import { usePyEffect } from "@summonpy/react-native-mage";
const PyJsi = NativeModules.PyJsiModule;

export default function App() {
  // const [result, setResult] = React.useState<number | undefined>();
  React.useEffect(() => {
    console.log({ PyJsi });
    if (PyJsi?.install) {
      PyJsi.install();
    }
  }, []);

  usePyEffect(() => {
    return `
      import numpy as np
    `;
  }, []);

  return (
    <View style={styles.container}>{/* <Text>Result: {result}</Text> */}</View>
  );
}

const styles = StyleSheet.create({
  container: {
    flex: 1,
    alignItems: "center",
    justifyContent: "center",
  },
  box: {
    width: 60,
    height: 60,
    marginVertical: 20,
  },
});
