import { useState } from 'react';
import { Card } from '../components/Card';
import { FilePicker } from '../components/FilePicker';
import { Select } from '../components/Select';
import { Input } from '../components/Input';
import { Button } from '../components/Button';
import { LogPanel } from '../components/LogPanel';
import { dumpFile } from '../lib/cli';
import type { LogEntry } from '../types';
import { FileCode, Download } from 'lucide-react';

const formatOptions = [
  { value: 'hex', label: 'Hexadecimal (Hex)' },
  { value: 'binary', label: 'Binary' },
  { value: 'base64', label: 'Base64' },
];

export function Dump() {
  const [inputFile, setInputFile] = useState('');
  const [format, setFormat] = useState<'hex' | 'binary' | 'base64'>('hex');
  const [maxBytes, setMaxBytes] = useState('4096');
  const [showOffset, setShowOffset] = useState(true);
  const [showAscii, setShowAscii] = useState(true);
  const [isProcessing, setIsProcessing] = useState(false);
  const [logs, setLogs] = useState<LogEntry[]>([]);
  const [output, setOutput] = useState('');

  const addLog = (level: LogEntry['level'], message: string) => {
    setLogs((prev) => [...prev, { timestamp: new Date(), level, message }]);
  };

  const handleDump = async () => {
    if (!inputFile) {
      addLog('error', 'Please select a file to dump');
      return;
    }

    setIsProcessing(true);
    setLogs([]);
    setOutput('');
    addLog('info', 'Starting file dump...');

    try {
      const result = await dumpFile(
        {
          input: inputFile,
          format,
          maxBytes: maxBytes ? parseInt(maxBytes) : undefined,
          showOffset,
          showAscii: format === 'hex' ? showAscii : undefined,
        },
        (log) => {
          addLog(log.level, log.message);
        }
      );

      if (result.success && result.stdout) {
        addLog('success', 'File dumped successfully!');
        setOutput(result.stdout);
      } else {
        const errorMsg = result.error || result.stderr || 'Dump failed';
        addLog('error', errorMsg);
      }
    } catch (err) {
      const errorMsg = err instanceof Error ? err.message : 'Unknown error';
      addLog('error', errorMsg);
    } finally {
      setIsProcessing(false);
    }
  };

  const handleCopyOutput = () => {
    if (output) {
      navigator.clipboard.writeText(output);
      addLog('success', 'Output copied to clipboard');
    }
  };

  const handleDownloadOutput = () => {
    if (output) {
      const blob = new Blob([output], { type: 'text/plain' });
      const url = URL.createObjectURL(blob);
      const a = document.createElement('a');
      a.href = url;
      a.download = `dump_${format}_${Date.now()}.txt`;
      document.body.appendChild(a);
      a.click();
      document.body.removeChild(a);
      URL.revokeObjectURL(url);
      addLog('success', 'Output downloaded');
    }
  };

  return (
    <div className="space-y-6">
      <div>
        <h1 className="text-3xl font-bold">Dump File</h1>
        <p className="text-base-content/70 mt-2">
          View file contents in hex, binary, or base64 format
        </p>
      </div>

      <Card title="File Selection">
        <div className="space-y-4">
          <div>
            <label className="block text-sm font-medium mb-2">Input File</label>
            <FilePicker value={inputFile} onChange={setInputFile} />
          </div>
        </div>
      </Card>

      <Card title="Dump Settings">
        <div className="space-y-4">
          <div>
            <label className="block text-sm font-medium mb-2">Output Format</label>
            <Select
              options={formatOptions}
              value={format}
              onChange={(e) => setFormat(e.target.value as 'hex' | 'binary' | 'base64')}
            />
          </div>

          <div className="grid grid-cols-1 md:grid-cols-2 gap-4">
            <div>
              <label className="block text-sm font-medium mb-2">
                Max Bytes (0 = all)
              </label>
              <Input
                type="number"
                min="0"
                value={maxBytes}
                onChange={(e) => setMaxBytes(e.target.value)}
                placeholder="4096"
              />
            </div>
          </div>

          {format === 'hex' && (
            <div className="space-y-2">
              <label className="flex items-center gap-2">
                <input
                  type="checkbox"
                  className="checkbox checkbox-primary"
                  checked={showOffset}
                  onChange={(e) => setShowOffset(e.target.checked)}
                />
                <span className="text-sm">Show byte offset</span>
              </label>
              <label className="flex items-center gap-2">
                <input
                  type="checkbox"
                  className="checkbox checkbox-primary"
                  checked={showAscii}
                  onChange={(e) => setShowAscii(e.target.checked)}
                />
                <span className="text-sm">Show ASCII representation</span>
              </label>
            </div>
          )}

          <Button
            onClick={handleDump}
            disabled={!inputFile || isProcessing}
            loading={isProcessing}
            className="w-full"
          >
            <FileCode className="w-4 h-4 mr-2" />
            {isProcessing ? 'Dumping...' : 'Dump File'}
          </Button>
        </div>
      </Card>

      {logs.length > 0 && (
        <Card title="Logs">
          <LogPanel logs={logs} maxHeight="200px" />
        </Card>
      )}

      {output && (
        <Card title="Dump Output">
          <div className="space-y-4">
            <div className="flex gap-2 justify-end">
              <Button variant="outline" size="sm" onClick={handleCopyOutput}>
                Copy
              </Button>
              <Button variant="outline" size="sm" onClick={handleDownloadOutput}>
                <Download className="w-4 h-4 mr-1" />
                Download
              </Button>
            </div>
            <div className="mockup-code bg-base-300 text-left">
              <pre className="text-sm leading-relaxed overflow-x-auto p-4 max-h-96 overflow-y-auto">
                <code className="text-base-content font-mono whitespace-pre">{output}</code>
              </pre>
            </div>
          </div>
        </Card>
      )}
    </div>
  );
}

