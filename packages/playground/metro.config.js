const MetroSymlinksResolver = require("@rnx-kit/metro-resolver-symlinks");
const { makeMetroConfig } = require("@rnx-kit/metro-config");

const { MetroSerializer } = require("@rnx-kit/metro-serializer");

const getRepoRoot = () => {
  return path.dirname(
    findUp.sync("pnpm-workspace.yaml", { cwd: process.cwd() })
  );
};

const {
  CyclicDependencies,
} = require("@rnx-kit/metro-plugin-cyclic-dependencies-detector");
const {
  DuplicateDependencies,
} = require("@rnx-kit/metro-plugin-duplicates-checker");

const path = require("path");
const findUp = require("find-up");

module.exports = makeMetroConfig({
  resolver: {
    resolveRequest: MetroSymlinksResolver(),
  },
  serializer: {
    customSerializer: MetroSerializer([
      CyclicDependencies(),
      DuplicateDependencies(),
    ]),
  },
  // useWatchman: false,
  watchFolders: [getRepoRoot()],
});
