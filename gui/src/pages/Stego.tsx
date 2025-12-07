import { useState } from 'react';
import { Card } from '../components/Card';
import { FilePicker } from '../components/FilePicker';
import { Input } from '../components/Input';
import { Button } from '../components/Button';
import { LogPanel } from '../components/LogPanel';
import { ProgressBar } from '../components/ProgressBar';
import { stegoHide, stegoExtract } from '../lib/cli';
import type { LogEntry } from '../types';
import { save } from '@tauri-apps/plugin-dialog';
import { FileUp } from 'lucide-react';

export function Stego() {
  const [mode, setMode] = useState<'hide' | 'extract'>('hide');
  const [container, setContainer] = useState('');
  const [payload, setPayload] = useState('');
  const [output, setOutput] = useState('');
  const [bits, setBits] = useState(2); // Bits per channel (1-8)
  const [isProcessing, setIsProcessing] = useState(false);
  const [progress, setProgress] = useState(0);
  const [logs, setLogs] = useState<LogEntry[]>([]);

  const addLog = (level: LogEntry['level'], message: string) => {
    setLogs((prev) => [...prev, { timestamp: new Date(), level, message }]);
  };

  const handleBrowseOutputHide = async () => {
    const selected = await save({
      title: 'Save stego container as',
      defaultPath: 'output_stego.png',
      filters: [{
        name: 'Image Files',
        extensions: ['png', 'bmp']
      }]
    });

    if (selected) {
      setOutput(selected);
    }
  };

  const handleBrowseOutputExtract = async () => {
    const selected = await save({
      title: 'Save extracted payload as',
      defaultPath: 'extracted_payload.txt',
    });

    if (selected) {
      setOutput(selected);
    }
  };

  const handleHide = async () => {
    if (!container || !payload || !output) {
      addLog('error', 'Please fill all required fields');
      return;
    }

    setIsProcessing(true);
    setProgress(0);
    addLog('info', 'Hiding payload in container...');

    try {
      const result = await stegoHide({
        container,
        payload,
        output,
        bits,
      });

      setProgress(100);

      if (result.success) {
        addLog('success', 'Payload hidden successfully!');
        addLog('info', result.stdout);
      } else {
        addLog('error', 'Hide operation failed: ' + (result.error || result.stderr));
      }
    } catch (error) {
      addLog('error', 'Unexpected error: ' + String(error));
    } finally {
      setIsProcessing(false);
    }
  };

  const handleExtract = async () => {
    if (!container || !output) {
      addLog('error', 'Please fill all required fields');
      return;
    }

    setIsProcessing(true);
    setProgress(0);
    addLog('info', 'Extracting payload from container...');

    try {
      const result = await stegoExtract({
        container,
        output,
        bits,
      });

      setProgress(100);

      if (result.success) {
        addLog('success', 'Payload extracted successfully!');
        addLog('info', result.stdout);
      } else {
        addLog('error', 'Extract operation failed: ' + (result.error || result.stderr));
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
        <h1 className="text-3xl font-bold">Steganography</h1>
        <p className="text-muted-foreground mt-2">
          Hide or extract secret data within container files (images, audio, etc.)
        </p>
      </div>

      <Card title="Mode Selection">
        <div className="flex gap-4">
          <Button
            variant={mode === 'hide' ? 'default' : 'outline'}
            onClick={() => setMode('hide')}
          >
            Hide Data
          </Button>
          <Button
            variant={mode === 'extract' ? 'default' : 'outline'}
            onClick={() => setMode('extract')}
          >
            Extract Data
          </Button>
        </div>
      </Card>

      {mode === 'hide' ? (
        <>
          <Card title="Hide Configuration">
            <div className="space-y-4">
              <div>
                <label className="block text-sm font-medium mb-2">Container File (Image/Audio)</label>
                <FilePicker value={container} onChange={setContainer} accept={['png', 'jpg', 'bmp', 'wav']} />
              </div>

              <div>
                <label className="block text-sm font-medium mb-2">Payload File (Data to Hide)</label>
                <FilePicker value={payload} onChange={setPayload} />
              </div>

              <div>
                <label className="block text-sm font-medium mb-2">Output File</label>
                <div className="flex gap-2">
                  <Input
                    type="text"
                    value={output}
                    onChange={(e) => setOutput(e.target.value)}
                    placeholder="output_stego.png"
                    className="flex-1"
                  />
                  <Button
                    type="button"
                    variant="outline"
                    onClick={handleBrowseOutputHide}
                    disabled={isProcessing}
                  >
                    <FileUp className="w-4 h-4" />
                  </Button>
                </div>
              </div>

              <div>
                <label className="block text-sm font-medium mb-2">Bits Per Channel (1-8)</label>
                <Input
                  type="number"
                  min={1}
                  max={8}
                  value={bits}
                  onChange={(e) => setBits(Number(e.target.value))}
                  placeholder="2"
                />
                <p className="text-sm text-muted-foreground mt-1">
                  Higher values = more capacity but more visible changes
                </p>
              </div>
            </div>
          </Card>

          <Button onClick={handleHide} disabled={isProcessing} size="lg">
            {isProcessing ? 'Hiding...' : 'Hide Payload'}
          </Button>
        </>
      ) : (
        <>
          <Card title="Extract Configuration">
            <div className="space-y-4">
              <div>
                <label className="block text-sm font-medium mb-2">Container File (with hidden data)</label>
                <FilePicker value={container} onChange={setContainer} accept={['png', 'jpg', 'bmp', 'wav']} />
              </div>

              <div>
                <label className="block text-sm font-medium mb-2">Output File (Extracted Data)</label>
                <div className="flex gap-2">
                  <Input
                    type="text"
                    value={output}
                    onChange={(e) => setOutput(e.target.value)}
                    placeholder="extracted_payload.txt"
                    className="flex-1"
                  />
                  <Button
                    type="button"
                    variant="outline"
                    onClick={handleBrowseOutputExtract}
                    disabled={isProcessing}
                  >
                    <FileUp className="w-4 h-4" />
                  </Button>
                </div>
              </div>

              <div>
                <label className="block text-sm font-medium mb-2">Bits Per Channel (1-8)</label>
                <Input
                  type="number"
                  min={1}
                  max={8}
                  value={bits}
                  onChange={(e) => setBits(Number(e.target.value))}
                  placeholder="2"
                />
                <p className="text-sm text-muted-foreground mt-1">
                  Must match the value used during hiding
                </p>
              </div>
            </div>
          </Card>

          <Button onClick={handleExtract} disabled={isProcessing} size="lg">
            {isProcessing ? 'Extracting...' : 'Extract Payload'}
          </Button>
        </>
      )}

      {isProcessing && (
        <Card title="Progress">
          <ProgressBar value={progress} />
        </Card>
      )}

      {logs.length > 0 && <LogPanel logs={logs} />}
    </div>
  );
}
