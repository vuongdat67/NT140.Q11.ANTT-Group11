import { useState } from 'react';
import { Card } from '../components/Card';
import { FilePicker } from '../components/FilePicker';
import { Input } from '../components/Input';
import { Select } from '../components/Select';
import { Button } from '../components/Button';
import { LogPanel } from '../components/LogPanel';
import { ProgressBar } from '../components/ProgressBar';
import { encryptFile } from '../lib/cli';
import { usePageState } from '../lib/usePageState';
import { addRecentFile, updateOperationStats } from '../lib/preferences';
import type { Algorithm, Mode, KDFAlgorithm, LogEntry } from '../types';
import { Eye, EyeOff } from 'lucide-react';

const algorithmOptions = [
  // Symmetric GCM modes (Recommended)
  { value: 'aes-256-gcm', label: 'AES-256-GCM (Recommended)' },
  { value: 'aes-192-gcm', label: 'AES-192-GCM' },
  { value: 'aes-128-gcm', label: 'AES-128-GCM' },
  { value: 'chacha20-poly1305', label: 'ChaCha20-Poly1305 (Fast)' },
  { value: 'serpent-256-gcm', label: 'Serpent-256-GCM' },
  { value: 'twofish-256-gcm', label: 'Twofish-256-GCM' },
  { value: 'twofish-192-gcm', label: 'Twofish-192-GCM' },
  { value: 'twofish-128-gcm', label: 'Twofish-128-GCM' },
  { value: 'camellia-256-gcm', label: 'Camellia-256-GCM' },
  { value: 'camellia-192-gcm', label: 'Camellia-192-GCM' },
  { value: 'camellia-128-gcm', label: 'Camellia-128-GCM' },
  { value: 'aria-256-gcm', label: 'ARIA-256-GCM' },
  { value: 'aria-192-gcm', label: 'ARIA-192-GCM' },
  { value: 'aria-128-gcm', label: 'ARIA-128-GCM' },
  { value: 'sm4-gcm', label: 'SM4-GCM (Chinese Standard)' },
  // AES other modes
  { value: 'aes-256-cbc', label: 'AES-256-CBC' },
  { value: 'aes-192-cbc', label: 'AES-192-CBC' },
  { value: 'aes-128-cbc', label: 'AES-128-CBC' },
  { value: 'aes-256-ctr', label: 'AES-256-CTR' },
  { value: 'aes-192-ctr', label: 'AES-192-CTR' },
  { value: 'aes-128-ctr', label: 'AES-128-CTR' },
  { value: 'aes-256-cfb', label: 'AES-256-CFB' },
  { value: 'aes-192-cfb', label: 'AES-192-CFB' },
  { value: 'aes-128-cfb', label: 'AES-128-CFB' },
  { value: 'aes-256-ofb', label: 'AES-256-OFB' },
  { value: 'aes-192-ofb', label: 'AES-192-OFB' },
  { value: 'aes-128-ofb', label: 'AES-128-OFB' },
  { value: 'aes-256-ecb', label: 'AES-256-ECB' },
  { value: 'aes-192-ecb', label: 'AES-192-ECB' },
  { value: 'aes-128-ecb', label: 'AES-128-ECB' },
  { value: 'aes-256-xts', label: 'AES-256-XTS' },
  { value: 'aes-128-xts', label: 'AES-128-XTS' },
  { value: '3des', label: '3DES (Legacy)' },
  // Asymmetric
  { value: 'rsa-4096', label: 'RSA-4096 (Asymmetric)' },
  { value: 'rsa-3072', label: 'RSA-3072 (Asymmetric)' },
  { value: 'rsa-2048', label: 'RSA-2048 (Asymmetric)' },
  { value: 'ecc-p521', label: 'ECC-P521 (Asymmetric)' },
  { value: 'ecc-p384', label: 'ECC-P384 (Asymmetric)' },
  { value: 'ecc-p256', label: 'ECC-P256 (Asymmetric)' },
  // Post-Quantum
  { value: 'kyber-1024-hybrid', label: 'Kyber-1024-Hybrid (Post-Quantum)' },
  { value: 'kyber-768-hybrid', label: 'Kyber-768-Hybrid (Post-Quantum)' },
  { value: 'kyber-512-hybrid', label: 'Kyber-512-Hybrid (Post-Quantum)' },
  // Classical (Educational)
  { value: 'caesar', label: 'Caesar (Classical - Weak)' },
  { value: 'vigenere', label: 'Vigenere (Classical - Weak)' },
  { value: 'playfair', label: 'Playfair (Classical - Weak)' },
  { value: 'substitution', label: 'Substitution (Classical - Weak)' },
  { value: 'hill', label: 'Hill (Classical - Weak)' },
];

const modeOptions = [
  { value: 'basic', label: 'Basic' },
  { value: 'standard', label: 'Standard' },
  { value: 'advanced', label: 'Advanced' },
];

const kdfOptions = [
  { value: 'argon2id', label: 'Argon2id (Recommended)' },
  { value: 'argon2i', label: 'Argon2i' },
  { value: 'pbkdf2-sha512', label: 'PBKDF2-SHA512' },
  { value: 'pbkdf2-sha256', label: 'PBKDF2-SHA256' },
  { value: 'scrypt', label: 'Scrypt' },
];

export function Encrypt() {
  // Use persisted state for form fields (but not sensitive data like passwords)
  const [inputFile, setInputFile] = usePageState('encrypt_inputFile', '');
  const [outputFile, setOutputFile] = usePageState('encrypt_outputFile', '');
  const [algorithm, setAlgorithm] = usePageState<Algorithm>('encrypt_algorithm', 'aes-256-gcm');
  const [mode, setMode] = usePageState<Mode>('encrypt_mode', 'standard');
  const [kdf, setKdf] = usePageState<KDFAlgorithm>('encrypt_kdf', 'argon2id');
  const [compression, setCompression] = usePageState('encrypt_compression', 'zlib');
  const [enableCompression, setEnableCompression] = usePageState('encrypt_enableCompression', false);
  const [secureDelete, setSecureDelete] = usePageState('encrypt_secureDelete', false);
  
  // Don't persist sensitive data
  const [password, setPassword] = useState('');
  const [confirmPassword, setConfirmPassword] = useState('');
  const [showPassword, setShowPassword] = useState(false);
  const [showConfirmPassword, setShowConfirmPassword] = useState(false);
  const [isProcessing, setIsProcessing] = useState(false);
  const [progress, setProgress] = useState(0);
  const [logs, setLogs] = useState<LogEntry[]>([]);

  const addLog = (level: LogEntry['level'], message: string) => {
    setLogs((prev) => [...prev, { timestamp: new Date(), level, message }]);
  };

  const handleEncrypt = async () => {
    // Validation
    if (!inputFile) {
      addLog('error', 'Please select an input file');
      return;
    }
    if (!outputFile) {
      addLog('error', 'Please specify output file path');
      return;
    }
    if (!password) {
      addLog('error', 'Please enter a password');
      return;
    }
    if (password !== confirmPassword) {
      addLog('error', 'Passwords do not match');
      return;
    }
    if (password.length < 8) {
      addLog('error', 'Password must be at least 8 characters');
      return;
    }

    setIsProcessing(true);
    setProgress(0);
    addLog('info', 'Starting encryption...');
    addLog('info', `Algorithm: ${algorithm}`);
    addLog('info', `Mode: ${mode}`);
    addLog('info', `KDF: ${kdf}`);
    if (enableCompression) {
      addLog('info', `Compression: ${compression.toUpperCase()}`);
    } else {
      addLog('info', 'Compression: Disabled');
    }

    try {
      const result = await encryptFile({
        input: inputFile,
        output: outputFile,
        algorithm,
        password,
        mode,
        compression: enableCompression ? compression : undefined,
        kdf,
      });

      setProgress(100);

      if (result.success) {
        addLog('success', 'File encrypted successfully!');
        if (result.stdout) addLog('info', result.stdout);
        // Add to recent files
        if (outputFile) {
          addRecentFile(outputFile, 'encrypt', true);
        }
        // Update operation stats
        updateOperationStats('encrypt', true, 1, 0);
      } else {
        addLog('error', 'Encryption failed: ' + (result.error || result.stderr));
        if (result.stdout) addLog('info', 'CLI output: ' + result.stdout);
        // Add to recent files even if failed
        if (outputFile) {
          addRecentFile(outputFile, 'encrypt', false);
        }
        // Update operation stats
        updateOperationStats('encrypt', false, 1, 0);
      }
    } catch (error) {
      addLog('error', 'Unexpected error: ' + String(error));
    } finally {
      setIsProcessing(false);
    }
  };

  const handleInputChange = (path: string) => {
    setInputFile(path);
    if (!outputFile) {
      setOutputFile(path + '.enc');
    }
  };

  return (
    <div className="space-y-6">
      <div>
        <h1 className="text-3xl font-bold">Encrypt File</h1>
        <p className="text-muted-foreground mt-2">
          Securely encrypt your files with strong encryption algorithms
        </p>
      </div>

      <Card title="File Selection">
        <div className="space-y-4">
          <div>
            <label className="block text-sm font-medium mb-2">Input File</label>
            <FilePicker value={inputFile} onChange={handleInputChange} />
          </div>

          <div>
            <label className="block text-sm font-medium mb-2">Output File</label>
            <Input
              type="text"
              value={outputFile}
              onChange={(e) => setOutputFile(e.target.value)}
              placeholder="encrypted_file.enc"
            />
          </div>
        </div>
      </Card>

      <Card title="Encryption Settings">
        <div className="grid grid-cols-1 md:grid-cols-2 gap-4">
          <div>
            <label className="block text-sm font-medium mb-2">Algorithm</label>
            <Select
              options={algorithmOptions}
              value={algorithm}
              onChange={(e) => setAlgorithm(e.target.value as Algorithm)}
            />
          </div>

          <div>
            <label className="block text-sm font-medium mb-2">Mode</label>
            <Select
              options={modeOptions}
              value={mode}
              onChange={(e) => setMode(e.target.value as Mode)}
            />
          </div>

          <div>
            <label className="block text-sm font-medium mb-2">Key Derivation Function</label>
            <Select
              options={kdfOptions}
              value={kdf}
              onChange={(e) => setKdf(e.target.value as KDFAlgorithm)}
            />
          </div>

          <div className="md:col-span-2">
            <div className="grid grid-cols-1 md:grid-cols-2 gap-4">
              <div>
                <label className="block text-sm font-medium mb-2">Password</label>
                <div className="relative">
                  <Input
                    type={showPassword ? 'text' : 'password'}
                    value={password}
                    onChange={(e) => setPassword(e.target.value)}
                    placeholder="Enter password (min 8 chars)"
                    className="pr-10"
                  />
                  <button
                    type="button"
                    onClick={() => setShowPassword(!showPassword)}
                    className="absolute right-2 top-1/2 -translate-y-1/2 text-muted-foreground hover:text-foreground"
                  >
                    {showPassword ? <EyeOff className="w-4 h-4" /> : <Eye className="w-4 h-4" />}
                  </button>
                </div>
              </div>

              <div>
                <label className="block text-sm font-medium mb-2">Confirm Password</label>
                <div className="relative">
                  <Input
                    type={showConfirmPassword ? 'text' : 'password'}
                    value={confirmPassword}
                    onChange={(e) => setConfirmPassword(e.target.value)}
                    placeholder="Confirm password"
                    className="pr-10"
                  />
                  <button
                    type="button"
                    onClick={() => setShowConfirmPassword(!showConfirmPassword)}
                    className="absolute right-2 top-1/2 -translate-y-1/2 text-muted-foreground hover:text-foreground"
                  >
                    {showConfirmPassword ? <EyeOff className="w-4 h-4" /> : <Eye className="w-4 h-4" />}
                  </button>
                </div>
              </div>
            </div>
          </div>
        </div>

        <div className="mt-4 space-y-2">
          <label className="flex items-center gap-2">
            <input
              type="checkbox"
              checked={enableCompression}
              onChange={(e) => setEnableCompression(e.target.checked)}
              className="rounded"
            />
            <span className="text-sm">Enable compression before encryption</span>
          </label>
          
          {enableCompression && (
            <div className="ml-6">
              <Select
                options={[
                  { value: 'zlib', label: 'ZLIB (Fast)' },
                  { value: 'bzip2', label: 'BZIP2 (Balanced)' },
                  { value: 'lzma', label: 'LZMA (Best compression)' },
                ]}
                value={compression}
                onChange={(e) => setCompression(e.target.value)}
              />
            </div>
          )}

          <label className="flex items-center gap-2">
            <input
              type="checkbox"
              checked={secureDelete}
              onChange={(e) => setSecureDelete(e.target.checked)}
              className="rounded"
            />
            <span className="text-sm">Securely delete original file after encryption</span>
          </label>
        </div>
      </Card>

      <div className="flex gap-4">
        <Button onClick={handleEncrypt} disabled={isProcessing} size="lg">
          {isProcessing ? 'Encrypting...' : 'Encrypt File'}
        </Button>
        <Button
          variant="outline"
          onClick={() => {
            setInputFile('');
            setOutputFile('');
            setPassword('');
            setConfirmPassword('');
            setLogs([]);
            setProgress(0);
          }}
          disabled={isProcessing}
        >
          Clear
        </Button>
      </div>

      {isProcessing && (
        <Card title="Progress">
          <ProgressBar value={progress} />
        </Card>
      )}

      {logs.length > 0 && <LogPanel logs={logs} />}
    </div>
  );
}
