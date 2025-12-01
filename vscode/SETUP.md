# FileVault VSCode Extension

## Prerequisites

### 1. Install Node.js
Download and install Node.js from: https://nodejs.org/

Recommended: LTS version (20.x or later)

### 2. Verify Installation
```powershell
node --version
npm --version
```

## Build Instructions

```powershell
# Navigate to extension folder
cd vscode

# Install dependencies
npm install

# Compile TypeScript
npm run compile

# Package extension (optional)
npx @vscode/vsce package
```

## Development

```powershell
# Watch mode for development
npm run watch
```

Press F5 in VS Code to launch Extension Development Host.

## Files

- `package.json` - Extension manifest
- `src/extension.ts` - Main extension code
- `tsconfig.json` - TypeScript configuration
- `.vscode/` - VS Code development settings
