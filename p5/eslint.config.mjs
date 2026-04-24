import js from "@eslint/js";
import globals from "globals";
import { defineConfig } from "eslint/config";

export default defineConfig([
  { files: ["**/*.{js,mjs,cjs}"], plugins: { js }, extends: ["js/recommended"], languageOptions: { globals: globals.browser } },
  { files: ["**/*.js"], languageOptions: { sourceType: "script" } },
  {
    "plugins": [
      "p5js"
    ],

    "env": {
      "browser": true,
      "es2021": true,
      "p5js/p5": true
    },

    "extends": [
      "eslint:recommended",
      "plugin:p5js/p5"
    ]
  }
]);
