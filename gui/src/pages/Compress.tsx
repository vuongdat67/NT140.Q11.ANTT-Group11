import { useMemo, useState, useEffect } from 'react';
import { Card } from '../components/Card';
import { FilePicker } from '../components/FilePicker';
import { Input } from '../components/Input';
import { Select } from '../components/Select';
import { Button } from '../components/Button';
import { LogPanel } from '../components/LogPanel';
import { ProgressBar } from '../components/ProgressBar';
import { compressFile, decompressFile } from '../lib/cli';
import type { CompressionAlgorithm, LogEntry } from '../types';
import { getDefaults } from '../lib/preferences';

const algorithmOptions = [
  { value: 'LZMA', label: 'LZMA (Best compression)' },
  { value: 'ZLIB', label: 'ZLIB (Balanced)' },
  { value: 'BZIP2', label: 'BZIP2' },
];

const levelOptions = [
  { value: '1', label: '1 (Fastest)' },
  { value: '3', label: '3' },
  { value: '6', label: '6 (Default)' },
  { value: '9', label: '9 (Best)' },
];

export function Compress() {
  const defaults = useMemo(() => getDefaults(), []);
  const [mode, setMode] = useState<'compress' | 'decompress'>('compress');
  const [inputFile, setInputFile] = useState('');
  const [outputFile, setOutputFile] = useState('');
  const [algorithm, setAlgorithm] = useState<CompressionAlgorithm>((defaults.compression || 'lzma').toUpperCase() as CompressionAlgorithm);
  const [level, setLevel] = useState(6);
  const [autoDetect, setAutoDetect] = useState(true);
  const [isProcessing, setIsProcessing] = useState(false);
  const [progress, setProgress] = useState(0);
  const [logs, setLogs] = useState<LogEntry[]>([]);

  // Sync with default compression from Settings
  useEffect(() => {
    const handleDefaultsChange = () => {
      const latest = getDefaults();
      setAlgorithm((latest.compression || 'lzma').toUpperCase() as CompressionAlgorithm);
    };

    window.addEventListener('defaultsChanged', handleDefaultsChange);

    return () => {
      window.removeEventListener('defaultsChanged', handleDefaultsChange);
    };
  }, []);

  const addLog = (level: LogEntry['level'], message: string) => {
    setLogs((prev) => [...prev, { timestamp: new Date(), level, message }]);
  };

  const handleCompress = async () => {
    if (!inputFile || !outputFile) {
      addLog('error', 'Please fill all required fields');
      return;
    }

    setIsProcessing(true);
    setProgress(0);
    addLog('info', `Compressing with ${algorithm}...`);

    try {
      const result = await compressFile({
        input: inputFile,
        output: outputFile,
        algorithm,
        level,
      });

      setProgress(100);

      if (result.success) {
        addLog('success', 'File compressed successfully!');
        addLog('info', result.stdout);
      } else {
        addLog('error', 'Compression failed: ' + (result.error || result.stderr));
      }
    } catch (error) {
      addLog('error', 'Unexpected error: ' + String(error));
    } finally {
      setIsProcessing(false);
    }
  };

  const handleDecompress = async () => {
    if (!inputFile || !outputFile) {
      addLog('error', 'Please fill all required fields');
      return;
    }

    setIsProcessing(true);
    setProgress(0);
    addLog('info', 'Decompressing...');

    try {
      const result = await decompressFile({
        input: inputFile,
        output: outputFile,
        algorithm: autoDetect ? undefined : algorithm,
        autoDetect,
      });

      setProgress(100);

      if (result.success) {
        addLog('success', 'File decompressed successfully!');
        addLog('info', result.stdout);
      } else {
        addLog('error', 'Decompression failed: ' + (result.error || result.stderr));
      }
    } catch (error) {
      addLog('error', 'Unexpected error: ' + String(error));
    } finally {
      setIsProcessing(false);
    }
  };

  const handleInputChange = (path: string) => {
    setInputFile(path);
    if (mode === 'compress' && (!outputFile || outputFile === '' || outputFile.endsWith('.lzma') || outputFile.endsWith('.zlib') || outputFile.endsWith('.bzip2'))) {
      setOutputFile(path ? `${path}.${algorithm.toLowerCase()}` : '');
    } else if (mode === 'decompress' && (!outputFile || outputFile === '')) {
      const extensions = ['.lzma', '.zlib', '.bzip2'];
      let output = path;
      for (const ext of extensions) {
        if (path.toLowerCase().endsWith(ext)) {
          output = path.slice(0, -ext.length);
          break;
        }
      }
      setOutputFile(output || path);
    }
  };

  const handleAlgorithmChange = (value: CompressionAlgorithm) => {
    setAlgorithm(value);
    if (mode === 'compress' && inputFile) {
      setOutputFile(`${inputFile}.${value.toLowerCase()}`);
    }
  };

  return (
    <div className="space-y-6">
      <div>
        <h1 className="text-3xl font-bold">Compress / Decompress</h1>
        <p className="text-muted-foreground mt-2">
          Compress or decompress files with various algorithms
        </p>
      </div>

      <Card title="Mode Selection">
        <div className="flex gap-4">
          <Button
            variant={mode === 'compress' ? 'default' : 'outline'}
            onClick={() => setMode('compress')}
          >
            Compress
          </Button>
          <Button
            variant={mode === 'decompress' ? 'default' : 'outline'}
            onClick={() => setMode('decompress')}
          >
            Decompress
          </Button>
        </div>
      </Card>

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
              placeholder={mode === 'compress' ? 'compressed.lzma' : 'decompressed.txt'}
            />
          </div>
        </div>
      </Card>

      {mode === 'compress' && (
        <Card title="Compression Settings">
          <div className="grid grid-cols-1 md:grid-cols-2 gap-4">
            <div>
                <label className="block text-sm font-medium mb-2">Algorithm</label>
                <Select
                  options={algorithmOptions}
                  value={algorithm}
                  onChange={(e) => handleAlgorithmChange(e.target.value as CompressionAlgorithm)}
                />
            </div>

            <div>
              <label className="block text-sm font-medium mb-2">Compression Level</label>
              <Select
                options={levelOptions}
                value={level.toString()}
                onChange={(e) => setLevel(Number(e.target.value))}
              />
            </div>
          </div>
        </Card>
      )}

      {mode === 'decompress' && (
        <Card title="Decompression Settings">
          <div className="space-y-4">
            <label className="flex items-center gap-2">
              <input
                type="checkbox"
                checked={autoDetect}
                onChange={(e) => setAutoDetect(e.target.checked)}
                className="rounded"
              />
              <span className="text-sm">Auto-detect algorithm from file (recommended)</span>
            </label>
            {!autoDetect && (
              <div>
                <label className="block text-sm font-medium mb-2">Algorithm</label>
                <Select
                  options={algorithmOptions}
                  value={algorithm}
                  onChange={(e) => handleAlgorithmChange(e.target.value as CompressionAlgorithm)}
                />
              </div>
            )}
          </div>
        </Card>
      )}

      <div className="flex gap-4">
        <Button
          onClick={mode === 'compress' ? handleCompress : handleDecompress}
          disabled={isProcessing}
          size="lg"
        >
          {isProcessing
            ? mode === 'compress'
              ? 'Compressing...'
              : 'Decompressing...'
            : mode === 'compress'
            ? 'Compress File'
            : 'Decompress File'}
        </Button>
        <Button
          variant="outline"
          onClick={() => {
            setInputFile('');
            setOutputFile('');
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
