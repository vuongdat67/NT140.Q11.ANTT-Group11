// CLI Types - using string to support all CLI algorithm variants
export type Algorithm = string;

export type HashAlgorithm = string;

export type CompressionAlgorithm = 
  | 'LZMA' 
  | 'ZLIB' 
  | 'BZIP2';

export type Mode = 'basic' | 'standard' | 'advanced';

export type KDFAlgorithm = string;

// Command Options
export interface EncryptOptions {
  input: string;
  output: string;
  algorithm: Algorithm;
  password?: string;
  keyfile?: string;
  mode?: Mode;
  compression?: string;
  kdf?: KDFAlgorithm;
  secureDelete?: boolean;
}

export interface DecryptOptions {
  input: string;
  output: string;
  password?: string;
  keyfile?: string;
  verify?: boolean;
}

export interface HashOptions {
  input: string;
  algorithm: HashAlgorithm;
  format?: 'hex' | 'base64' | 'binary';
}

export interface KeygenOptions {
  algorithm: Algorithm;
  output: string;
  keySize?: number;
  force?: boolean;
}

export interface SignOptions {
  input: string;
  output: string;
  privateKey: string;
  password?: string;
}

export interface VerifyOptions {
  input: string;
  signature: string;
  publicKey: string;
}

export interface StegoHideOptions {
  container: string;
  payload: string;
  output: string;
  bits?: number; // Bits per channel (1-4)
}

export interface StegoExtractOptions {
  container: string;
  output: string;
  bits?: number; // Bits per channel (1-4)
}

export interface ArchiveCreateOptions {
  files: string[];
  output: string;
  compression?: CompressionAlgorithm;
  level?: number;
  encrypt?: boolean;
  password?: string;
}

export interface ArchiveExtractOptions {
  input: string;
  output: string;
  password?: string;
}

export interface CompressOptions {
  input: string;
  output: string;
  algorithm: CompressionAlgorithm;
  level?: number;
}

export interface DecompressOptions {
  input: string;
  output: string;
  algorithm?: CompressionAlgorithm;
  autoDetect?: boolean;
}

// Command Result
export interface CommandResult {
  success: boolean;
  stdout: string;
  stderr: string;
  exitCode: number;
  error?: string;
}

// Log Entry
export interface LogEntry {
  timestamp: Date;
  level: 'info' | 'success' | 'warning' | 'error';
  message: string;
}

// UI State
export interface FileInfo {
  path: string;
  name: string;
  size: number;
}
