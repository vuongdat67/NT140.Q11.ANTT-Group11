import { useState } from 'react';
import { Card } from '../components/Card';
import { FilePicker } from '../components/FilePicker';
import { Select } from '../components/Select';
import { Button } from '../components/Button';
import { Input } from '../components/Input';
import { LogPanel } from '../components/LogPanel';
import { hashFile } from '../lib/cli';
import type { HashAlgorithm, LogEntry } from '../types';
import { Copy, Check } from 'lucide-react';

const algorithmOptions = [
  // SHA-2 family
  { value: 'sha256', label: 'SHA-256 (Recommended)' },
  { value: 'sha512', label: 'SHA-512' },
  { value: 'sha384', label: 'SHA-384' },
  { value: 'sha224', label: 'SHA-224' },
  { value: 'sha512-256', label: 'SHA-512/256' },
  // SHA-3 family
  { value: 'sha3-256', label: 'SHA3-256' },
  { value: 'sha3-384', label: 'SHA3-384' },
  { value: 'sha3-512', label: 'SHA3-512' },
  // BLAKE2
  { value: 'blake2b-512', label: 'BLAKE2b-512' },
  { value: 'blake2b-256', label: 'BLAKE2b-256' },
  { value: 'blake2s-256', label: 'BLAKE2s-256' },
  // Other modern
  { value: 'ripemd-160', label: 'RIPEMD-160' },
  { value: 'whirlpool', label: 'Whirlpool' },
  { value: 'tiger', label: 'Tiger' },
  { value: 'sm3', label: 'SM3 (Chinese Standard)' },
  { value: 'streebog-512', label: 'Streebog-512 (GOST)' },
  { value: 'streebog-256', label: 'Streebog-256 (GOST)' },
  // Legacy (weak)
  { value: 'sha1', label: 'SHA-1 (Legacy - Weak)' },
  { value: 'md5', label: 'MD5 (Legacy - Weak)' },
];

const formatOptions = [
  { value: 'hex', label: 'Hexadecimal' },
  { value: 'base64', label: 'Base64' },
  { value: 'binary', label: 'Binary' },
];

export function Hash() {
  const [inputFile, setInputFile] = useState('');
  const [algorithm, setAlgorithm] = useState<HashAlgorithm>('sha256');
  const [format, setFormat] = useState<'hex' | 'base64' | 'binary'>('hex');
  const [hashResult, setHashResult] = useState('');
  const [isProcessing, setIsProcessing] = useState(false);
  const [logs, setLogs] = useState<LogEntry[]>([]);
  const [copied, setCopied] = useState(false);

  const addLog = (level: LogEntry['level'], message: string) => {
    setLogs((prev) => [...prev, { timestamp: new Date(), level, message }]);
  };

  const handleHash = async () => {
    if (!inputFile) {
      addLog('error', 'Please select a file');
      return;
    }

    setIsProcessing(true);
    setHashResult('');
    addLog('info', `Computing ${algorithm} hash...`);

    try {
      const result = await hashFile({
        input: inputFile,
        algorithm,
        format,
      });

      if (result.success) {
        // CLI output format: "<HASH>  <filename>"
        // Binary format: "00101010 01100001 ... filename"
        // Extract hash (everything before the filename at the end)
        const output = result.stdout.trim();
        const lines = output.split('\n');
        const lastLine = lines[lines.length - 1];
        
        // Find the last occurrence of double-space or filename pattern
        // For binary format, take everything except the last token (filename)
        const tokens = lastLine.split(/\s+/);
        const hashValue = format === 'binary' 
          ? tokens.slice(0, -1).join(' ')  // All bytes except filename
          : tokens[0];  // For hex/base64, first token only
          
        setHashResult(hashValue);
        addLog('success', 'Hash computed successfully!');
        addLog('info', `Hash: ${hashValue}`);
      } else {
        addLog('error', 'Hash computation failed: ' + (result.error || result.stderr));
        if (result.stdout) addLog('info', 'CLI output: ' + result.stdout);
      }
    } catch (error) {
      addLog('error', 'Unexpected error: ' + String(error));
    } finally {
      setIsProcessing(false);
    }
  };

  const handleCopy = async () => {
    if (hashResult) {
      await navigator.clipboard.writeText(hashResult);
      setCopied(true);
      setTimeout(() => setCopied(false), 2000);
      addLog('info', 'Hash copied to clipboard');
    }
  };

  return (
    <div className="space-y-6">
      <div>
        <h1 className="text-3xl font-bold">Hash File</h1>
        <p className="text-muted-foreground mt-2">
          Compute cryptographic hash of files for integrity verification
        </p>
      </div>

      <Card title="File Selection">
        <div>
          <label className="block text-sm font-medium mb-2">Input File</label>
          <FilePicker value={inputFile} onChange={setInputFile} />
        </div>
      </Card>

      <Card title="Hash Settings">
        <div className="grid grid-cols-1 md:grid-cols-2 gap-4">
          <div>
            <label className="block text-sm font-medium mb-2">Algorithm</label>
            <Select
              options={algorithmOptions}
              value={algorithm}
              onChange={(e) => setAlgorithm(e.target.value as HashAlgorithm)}
            />
          </div>

          <div>
            <label className="block text-sm font-medium mb-2">Output Format</label>
            <Select
              options={formatOptions}
              value={format}
              onChange={(e) => setFormat(e.target.value as 'hex' | 'base64')}
            />
          </div>
        </div>
      </Card>

      <div className="flex gap-4">
        <Button onClick={handleHash} disabled={isProcessing} size="lg">
          {isProcessing ? 'Computing...' : 'Compute Hash'}
        </Button>
        <Button
          variant="outline"
          onClick={() => {
            setInputFile('');
            setHashResult('');
            setLogs([]);
          }}
          disabled={isProcessing}
        >
          Clear
        </Button>
      </div>

      {hashResult && (
        <Card title="Hash Result">
          <div className="space-y-4">
            <div className="flex items-center gap-2">
              <Input value={hashResult} readOnly className="font-mono text-xs" />
              <Button onClick={handleCopy} variant="outline" size="icon">
                {copied ? <Check className="w-4 h-4" /> : <Copy className="w-4 h-4" />}
              </Button>
            </div>
            <p className="text-xs text-muted-foreground">
              Algorithm: {algorithm} | Format: {format}
            </p>
          </div>
        </Card>
      )}

      {logs.length > 0 && <LogPanel logs={logs} />}
    </div>
  );
}
