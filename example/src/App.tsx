import * as React from 'react';

import { StyleSheet, View } from 'react-native';
// import { multiply } from 'react-native-pyjsi';
import { NativeModules } from 'react-native';

const PyJsi = NativeModules.PyJsiModule;

export default function App() {
  // const [result, setResult] = React.useState<number | undefined>();
  React.useEffect(() => {
    console.log({ PyJsi });
    if (PyJsi?.install) {
      PyJsi.install();
    }
  }, []);

  return (
    <View style={styles.container}>{/* <Text>Result: {result}</Text> */}</View>
  );
}

const styles = StyleSheet.create({
  container: {
    flex: 1,
    alignItems: 'center',
    justifyContent: 'center',
  },
  box: {
    width: 60,
    height: 60,
    marginVertical: 20,
  },
});
