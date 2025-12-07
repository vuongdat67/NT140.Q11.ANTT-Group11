import { useState } from 'react';
import { Card } from '../components/Card';
import { Input } from '../components/Input';
import { Select } from '../components/Select';
import { Button } from '../components/Button';
import { LogPanel } from '../components/LogPanel';
import { generateKeypair } from '../lib/cli';
import type { Algorithm, LogEntry } from '../types';
import { open } from '@tauri-apps/plugin-dialog';
import { FolderOpen } from 'lucide-react';

const algorithmOptions = [
  { value: 'RSA-4096', label: 'RSA-4096 (Recommended)' },
  { value: 'RSA-3072', label: 'RSA-3072' },
  { value: 'RSA-2048', label: 'RSA-2048' },
  { value: 'ECC-P256', label: 'ECC-P256' },
  { value: 'ECC-P384', label: 'ECC-P384' },
  { value: 'ECC-P521', label: 'ECC-P521' },
];

export function Keygen() {
  const [algorithm, setAlgorithm] = useState<Algorithm>('RSA-4096');
  const [outputPath, setOutputPath] = useState('');
  const [forceOverwrite, setForceOverwrite] = useState(false);
  const [isProcessing, setIsProcessing] = useState(false);
  const [logs, setLogs] = useState<LogEntry[]>([]);

  const addLog = (level: LogEntry['level'], message: string) => {
    setLogs((prev) => [...prev, { timestamp: new Date(), level, message }]);
  };

  const handleBrowseFolder = async () => {
    const selected = await open({
      directory: true,
      multiple: false,
      title: 'Select folder to save keys',
    });

    if (selected && typeof selected === 'string') {
      // Set output path with default key name
      const keyName = algorithm.toLowerCase().replace(/-/g, '_');
      setOutputPath(`${selected}\\${keyName}`);
    }
  };

  const handleGenerate = async () => {
    if (!outputPath) {
      addLog('error', 'Please specify output file prefix (e.g., C:\\keys\\mykey)');
      return;
    }

    setIsProcessing(true);
    addLog('info', `Generating ${algorithm} keypair...`);

    try {
      const result = await generateKeypair({
        algorithm,
        output: outputPath,
        force: forceOverwrite,
      });

      if (result.success) {
        addLog('success', 'Keypair generated successfully!');
        if (result.stdout) addLog('info', result.stdout);
      } else {
        addLog('error', 'Key generation failed: ' + (result.error || result.stderr));
        if (result.stdout) addLog('info', 'CLI output: ' + result.stdout);
      }
    } catch (error) {
      addLog('error', 'Unexpected error: ' + String(error));
    } finally {
      setIsProcessing(false);
    }
  };

  return (
    <div className="space-y-6">
      <div>
        <h1 className="text-3xl font-bold">Generate Keypair</h1>
        <p className="text-muted-foreground mt-2">
          Generate asymmetric keypairs for encryption and signing
        </p>
      </div>

      <Card title="Key Configuration">
        <div className="space-y-4">
          <div>
            <label className="block text-sm font-medium mb-2">Algorithm</label>
            <Select
              options={algorithmOptions}
              value={algorithm}
              onChange={(e) => setAlgorithm(e.target.value as Algorithm)}
            />
            <p className="text-xs text-muted-foreground mt-1">
              Key size is determined by the algorithm (RSA-4096 = 4096 bits)
            </p>
          </div>

          <div>
            <label className="block text-sm font-medium mb-2">Output File Prefix</label>
            <div className="flex gap-2">
              <Input
                type="text"
                value={outputPath}
                onChange={(e) => setOutputPath(e.target.value)}
                placeholder="C:\keys\mykey"
                className="flex-1"
              />
              <Button
                type="button"
                variant="outline"
                onClick={handleBrowseFolder}
                disabled={isProcessing}
              >
                <FolderOpen className="w-4 h-4" />
              </Button>
            </div>
            <p className="text-xs text-muted-foreground mt-1">
              Will create: {outputPath}.pub and {outputPath}.key
            </p>
          </div>

          <div>
            <label className="flex items-center gap-2">
              <input
                type="checkbox"
                checked={forceOverwrite}
                onChange={(e) => setForceOverwrite(e.target.checked)}
                className="rounded"
              />
              <span className="text-sm">Overwrite existing keys if present</span>
            </label>
          </div>
        </div>
      </Card>

      <Card title="Note">
        <p className="text-sm text-muted-foreground">
          Private keys will be generated unencrypted. Protect your private key file carefully and consider encrypting it separately if needed.
        </p>
      </Card>

      <div className="flex gap-4">
        <Button onClick={handleGenerate} disabled={isProcessing} size="lg">
          {isProcessing ? 'Generating...' : 'Generate Keypair'}
        </Button>
        <Button
          variant="outline"
          onClick={() => {
            setOutputPath('');
            setLogs([]);
          }}
          disabled={isProcessing}
        >
          Clear
        </Button>
      </div>

      {logs.length > 0 && <LogPanel logs={logs} />}
    </div>
  );
}
