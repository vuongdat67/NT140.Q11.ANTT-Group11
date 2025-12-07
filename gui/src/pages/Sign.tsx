import { useState } from 'react';
import { Card } from '../components/Card';
import { FilePicker } from '../components/FilePicker';
import { Input } from '../components/Input';
import { Button } from '../components/Button';
import { LogPanel } from '../components/LogPanel';
import { signFile } from '../lib/cli';
import type { LogEntry } from '../types';
import { save } from '@tauri-apps/plugin-dialog';
import { FileUp } from 'lucide-react';

export function Sign() {
  const [inputFile, setInputFile] = useState('');
  const [outputFile, setOutputFile] = useState('');
  const [privateKey, setPrivateKey] = useState('');
  const [password, setPassword] = useState('');
  const [isProcessing, setIsProcessing] = useState(false);
  const [logs, setLogs] = useState<LogEntry[]>([]);

  const addLog = (level: LogEntry['level'], message: string) => {
    setLogs((prev) => [...prev, { timestamp: new Date(), level, message }]);
  };

  const handleBrowseOutput = async () => {
    const selected = await save({
      title: 'Save signature as',
      defaultPath: 'signature.sign',
      filters: [{
        name: 'Signature Files',
        extensions: ['sign', 'sig']
      }]
    });

    if (selected) {
      setOutputFile(selected);
    }
  };

  const handleSign = async () => {
    if (!inputFile || !outputFile || !privateKey) {
      addLog('error', 'Please fill all required fields');
      return;
    }

    setIsProcessing(true);
    addLog('info', 'Signing file...');

    try {
      const result = await signFile({
        input: inputFile,
        output: outputFile,
        privateKey,
        password: password || undefined,
      });

      if (result.success) {
        addLog('success', 'File signed successfully!');
        addLog('info', result.stdout);
      } else {
        addLog('error', 'Signing failed: ' + (result.error || result.stderr));
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
        <h1 className="text-3xl font-bold">Sign File</h1>
        <p className="text-muted-foreground mt-2">
          Create digital signature for file authentication
        </p>
      </div>

      <Card title="File Selection">
        <div className="space-y-4">
          <div>
            <label className="block text-sm font-medium mb-2">Input File</label>
            <FilePicker value={inputFile} onChange={setInputFile} />
          </div>

          <div>
            <label className="block text-sm font-medium mb-2">Signature Output File</label>
            <div className="flex gap-2">
              <Input
                type="text"
                value={outputFile}
                onChange={(e) => setOutputFile(e.target.value)}
                placeholder="signature.sign"
                className="flex-1"
              />
              <Button
                type="button"
                variant="outline"
                onClick={handleBrowseOutput}
                disabled={isProcessing}
              >
                <FileUp className="w-4 h-4" />
              </Button>
            </div>
            <p className="text-xs text-muted-foreground mt-1">
              CLI creates .sign files by default
            </p>
          </div>

          <div>
            <label className="block text-sm font-medium mb-2">Private Key</label>
            <FilePicker value={privateKey} onChange={setPrivateKey} accept={['pem', 'key']} />
            <p className="text-xs text-muted-foreground mt-1">
              Use .key file generated from Keygen page (PEM format)
            </p>
          </div>

          <div>
            <label className="block text-sm font-medium mb-2">Private Key Password (if encrypted)</label>
            <Input
              type="password"
              value={password}
              onChange={(e) => setPassword(e.target.value)}
              placeholder="Enter password"
            />
          </div>
        </div>
      </Card>

      <div className="flex gap-4">
        <Button onClick={handleSign} disabled={isProcessing} size="lg">
          {isProcessing ? 'Signing...' : 'Sign File'}
        </Button>
        <Button
          variant="outline"
          onClick={() => {
            setInputFile('');
            setOutputFile('');
            setPrivateKey('');
            setPassword('');
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
