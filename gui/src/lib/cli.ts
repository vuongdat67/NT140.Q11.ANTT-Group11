import { invoke } from '@tauri-apps/api/core';
import type {
  EncryptOptions,
  DecryptOptions,
  HashOptions,
  KeygenOptions,
  SignOptions,
  VerifyOptions,
  StegoHideOptions,
  StegoExtractOptions,
  ArchiveCreateOptions,
  ArchiveExtractOptions,
  CompressOptions,
  DecompressOptions,
  CommandResult,
} from '../types';

/**
 * Execute FileVault CLI command via Tauri backend
 */
async function executeCommand(args: string[]): Promise<CommandResult> {
  try {
    const result = await invoke<CommandResult>('run_filevault_command', { args });
    return result;
  } catch (error) {
    return {
      success: false,
      stdout: '',
      stderr: String(error),
      exitCode: 1,
      error: String(error),
    };
  }
}

/**
 * Encrypt a file
 */
export async function encryptFile(options: EncryptOptions): Promise<CommandResult> {
  const args = [
    'encrypt',
    options.input,
  ];
  
  if (options.output) {
    args.push(options.output);
  }
  
  args.push('-a', options.algorithm.toLowerCase());

  if (options.password) {
    args.push('-p', options.password);
    args.push('-y'); // Skip weak password prompt
  }

  if (options.mode) {
    args.push('-m', options.mode);
  }

  if (options.compression) {
    args.push('--compression', options.compression.toLowerCase());
  }

  if (options.kdf) {
    args.push('--kdf', options.kdf.toLowerCase());
  }

  return executeCommand(args);
}

/**
 * Decrypt a file
 */
export async function decryptFile(options: DecryptOptions): Promise<CommandResult> {
  const args = [
    'decrypt',
    options.input,
  ];
  
  if (options.output) {
    args.push(options.output);
  }

  if (options.password) {
    args.push('-p', options.password);
  }

  if (options.keyfile) {
    args.push('-k', options.keyfile);
  }

  return executeCommand(args);
}

/**
 * Hash a file
 */
export async function hashFile(options: HashOptions): Promise<CommandResult> {
  const args = [
    'hash',
    options.input,
    '-a',
    options.algorithm.toLowerCase(),
  ];

  if (options.format) {
    args.push('--format', options.format.toLowerCase());
  }

  return executeCommand(args);
}

/**
 * Generate keypair
 */
export async function generateKeypair(options: KeygenOptions): Promise<CommandResult> {
  const args = [
    'keygen',
    '-a',
    options.algorithm.toLowerCase(),
    '-o',
    options.output,
  ];

  if (options.force) {
    args.push('-f');
  }

  return executeCommand(args);
}

/**
 * Sign a file
 */
export async function signFile(options: SignOptions): Promise<CommandResult> {
  const args = [
    'sign',
    options.input,
    options.privateKey,
    '-o',
    options.output,
  ];

  if (options.password) {
    args.push('-p', options.password);
  }

  return executeCommand(args);
}

/**
 * Verify a signature
 */
export async function verifySignature(options: VerifyOptions): Promise<CommandResult> {
  const args = [
    'verify',
    options.input,
    options.signature,
    options.publicKey,
  ];

  return executeCommand(args);
}

/**
 * Hide data in container (steganography)
 */
export async function stegoHide(options: StegoHideOptions): Promise<CommandResult> {
  const args = [
    'stego',
    'embed',
    options.payload,
    options.container,
    options.output,
  ];

  if (options.bits) {
    args.push('-b', options.bits.toString());
  }

  return executeCommand(args);
}

/**
 * Extract hidden data from container
 */
export async function stegoExtract(options: StegoExtractOptions): Promise<CommandResult> {
  const args = [
    'stego',
    'extract',
    options.container,
    options.output,
  ];

  if (options.bits) {
    args.push('-b', options.bits.toString());
  }

  return executeCommand(args);
}

/**
 * Create archive
 */
export async function createArchive(options: ArchiveCreateOptions): Promise<CommandResult> {
  const args = [
    'archive',
    'create',
    ...options.files,
    '-o',
    options.output,
  ];
  
  if (options.compression) {
    args.push('-c', options.compression.toLowerCase());
  }

  // Note: Archive CLI doesn't support compression level flag
  // Level is determined by the compression algorithm

  if (options.password) {
    args.push('-p', options.password);
  }

  return executeCommand(args);
}

/**
 * Extract archive
 */
export async function extractArchive(options: ArchiveExtractOptions): Promise<CommandResult> {
  const args = [
    'archive',
    'extract',
    options.input,
    '-o',
    options.output,
  ];

  if (options.password) {
    args.push('-p', options.password);
  }

  return executeCommand(args);
}

/**
 * Compress a file
 */
export async function compressFile(options: CompressOptions): Promise<CommandResult> {
  const args = [
    'compress',
    options.input,
    '-a',
    options.algorithm.toLowerCase(),
    '-o',
    options.output,
  ];

  if (options.level) {
    args.push('-l', options.level.toString());
  }

  return executeCommand(args);
}

/**
 * Decompress a file
 */
export async function decompressFile(options: DecompressOptions): Promise<CommandResult> {
  const args = [
    'decompress',
    options.input,
    '-o',
    options.output,
  ];

  return executeCommand(args);
}

/**
 * Benchmark cryptographic algorithms
 */
export async function benchmarkAlgorithm(
  options: {
    algorithm?: string;
    size?: number;
    iterations?: number;
    symmetric?: boolean;
    asymmetric?: boolean;
    pqc?: boolean;
    hash?: boolean;
    kdf?: boolean;
    compression?: boolean;
  },
  onLog?: (log: { level: 'info' | 'success' | 'warning' | 'error'; message: string }) => void
): Promise<CommandResult> {
  const args = ['benchmark'];

  if (options.algorithm) {
    args.push('-a', options.algorithm);
  } else {
    args.push('--all');
  }

  if (options.size) {
    args.push('-s', options.size.toString());
  }

  if (options.iterations) {
    args.push('-i', options.iterations.toString());
  }

  if (options.symmetric) args.push('--symmetric');
  if (options.asymmetric) args.push('--asymmetric');
  if (options.pqc) args.push('--pqc');
  if (options.hash) args.push('--hash');
  if (options.kdf) args.push('--kdf');
  if (options.compression) args.push('--compression');

  const result = await executeCommand(args);

  if (onLog && result.stdout) {
    result.stdout.split('\n').forEach((line) => {
      if (line.trim()) {
        onLog({ level: 'info', message: line });
      }
    });
  }

  return result;
}

/**
 * Get information about encrypted file
 */
export async function getFileInfo(
  options: { input: string; verbose?: boolean },
  onLog?: (log: { level: 'info' | 'success' | 'warning' | 'error'; message: string }) => void
): Promise<CommandResult> {
  const args = ['info', options.input];

  if (options.verbose) {
    args.push('-v');
  }

  const result = await executeCommand(args);

  if (onLog && result.stdout) {
    result.stdout.split('\n').forEach((line) => {
      if (line.trim()) {
        onLog({ level: 'info', message: line });
      }
    });
  }

  return result;
}
