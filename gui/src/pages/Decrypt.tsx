import { useState } from 'react';
import { Card } from '../components/Card';
import { FilePicker } from '../components/FilePicker';
import { Input } from '../components/Input';
import { Button } from '../components/Button';
import { LogPanel } from '../components/LogPanel';
import { ProgressBar } from '../components/ProgressBar';
import { decryptFile } from '../lib/cli';
import { usePageState } from '../lib/usePageState';
import { addRecentFile, updateOperationStats } from '../lib/preferences';
import type { LogEntry } from '../types';
import { Eye, EyeOff } from 'lucide-react';

export function Decrypt() {
  // Use persisted state for form fields
  const [inputFile, setInputFile] = usePageState('decrypt_inputFile', '');
  const [outputFile, setOutputFile] = usePageState('decrypt_outputFile', '');
  const [keyfile, setKeyfile] = usePageState('decrypt_keyfile', '');
  const [verify, setVerify] = usePageState('decrypt_verify', true);
  
  // Don't persist sensitive data
  const [password, setPassword] = useState('');
  const [showPassword, setShowPassword] = useState(false);
  const [isProcessing, setIsProcessing] = useState(false);
  const [progress, setProgress] = useState(0);
  const [logs, setLogs] = useState<LogEntry[]>([]);

  const addLog = (level: LogEntry['level'], message: string) => {
    setLogs((prev) => [...prev, { timestamp: new Date(), level, message }]);
  };

  const handleDecrypt = async () => {
    if (!inputFile) {
      addLog('error', 'Please select an encrypted file');
      return;
    }
    if (!outputFile) {
      addLog('error', 'Please specify output file path');
      return;
    }
    if (!password && !keyfile) {
      addLog('error', 'Please enter a password or select a keyfile');
      return;
    }

    setIsProcessing(true);
    setProgress(0);
    addLog('info', 'Starting decryption...');

    try {
      const result = await decryptFile({
        input: inputFile,
        output: outputFile,
        password,
        keyfile: keyfile || undefined,
        verify,
      });

      setProgress(100);

      if (result.success) {
        addLog('success', 'File decrypted successfully!');
        addLog('info', result.stdout);
        // Add to recent files
        if (outputFile) {
          addRecentFile(outputFile, 'decrypt', true);
        }
        // Update operation stats
        updateOperationStats('decrypt', true, 1, 0);
      } else {
        addLog('error', 'Decryption failed: ' + (result.error || result.stderr));
        // Add to recent files even if failed
        if (outputFile) {
          addRecentFile(outputFile, 'decrypt', false);
        }
        // Update operation stats
        updateOperationStats('decrypt', false, 1, 0);
      }
    } catch (error) {
      addLog('error', 'Unexpected error: ' + String(error));
    } finally {
      setIsProcessing(false);
    }
  };

  const handleInputChange = (path: string) => {
    setInputFile(path);
    // Always auto-update output file when input changes (like Encrypt page)
    // Remove .fvlt extension but preserve original extension
    // Example: "terminal.jpg.fvlt" -> "terminal.jpg"
    const knownExts = ['.fvlt', '.enc', '.encrypted', '.fv'];
    let output = path;
    for (const ext of knownExts) {
      if (path.toLowerCase().endsWith(ext.toLowerCase())) {
        output = path.slice(0, -ext.length);
        break;
      }
    }
    // If no known extension found, add .decrypted
    if (output === path && path) {
      output = `${path}.decrypted`;
    }
    setOutputFile(output);
  };

  return (
    <div className="space-y-6">
      <div>
        <h1 className="text-3xl font-bold">Decrypt File</h1>
        <p className="text-muted-foreground mt-2">
          Decrypt files that were encrypted with FileVault
        </p>
      </div>

      <Card title="File Selection">
        <div className="space-y-4">
          <div>
            <label className="block text-sm font-medium mb-2">Encrypted File</label>
            <FilePicker value={inputFile} onChange={handleInputChange} accept={['fvlt']} />
          </div>

          <div>
            <label className="block text-sm font-medium mb-2">Output File</label>
            <Input
              type="text"
              value={outputFile}
              onChange={(e) => setOutputFile(e.target.value)}
              placeholder="decrypted_file.txt"
            />
          </div>
        </div>
      </Card>

      <Card title="Decryption Settings">
        <div className="space-y-4">
          <div>
            <label className="block text-sm font-medium mb-2">Password</label>
            <div className="relative">
              <Input
                type={showPassword ? 'text' : 'password'}
                value={password}
                onChange={(e) => setPassword(e.target.value)}
                placeholder="Enter password"
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
            <label className="block text-sm font-medium mb-2">Or use Keyfile (optional)</label>
            <FilePicker
              value={keyfile}
              onChange={setKeyfile}
              placeholder="Select keyfile..."
            />
          </div>

          <label className="flex items-center gap-2">
            <input
              type="checkbox"
              checked={verify}
              onChange={(e) => setVerify(e.target.checked)}
              className="rounded"
            />
            <span className="text-sm">Verify signature (if present)</span>
          </label>
        </div>
      </Card>

      <div className="flex gap-4">
        <Button onClick={handleDecrypt} disabled={isProcessing} size="lg">
          {isProcessing ? 'Decrypting...' : 'Decrypt File'}
        </Button>
        <Button
          variant="outline"
          onClick={() => {
            setInputFile('');
            setOutputFile('');
            setPassword('');
            setKeyfile('');
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
