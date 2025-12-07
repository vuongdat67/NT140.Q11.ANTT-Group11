import { useState } from 'react';
import { Card } from '../components/Card';
import { FilePicker } from '../components/FilePicker';
import { Input } from '../components/Input';
import { Select } from '../components/Select';
import { Button } from '../components/Button';
import { LogPanel } from '../components/LogPanel';
import { ProgressBar } from '../components/ProgressBar';
import { createArchive, extractArchive } from '../lib/cli';
import type { CompressionAlgorithm, LogEntry } from '../types';
import { Plus, X, Files, FileUp } from 'lucide-react';
import { open, save } from '@tauri-apps/plugin-dialog';

const compressionOptions = [
  { value: 'LZMA', label: 'LZMA (Best compression)' },
  { value: 'ZLIB', label: 'ZLIB (Balanced)' },
  { value: 'BZIP3', label: 'BZIP3 (Fast, modern)' },
];

export function Archive() {
  const [mode, setMode] = useState<'create' | 'extract'>('create');
  const [files, setFiles] = useState<string[]>([]);
  const [newFile, setNewFile] = useState('');
  const [archivePath, setArchivePath] = useState('');
  const [extractPath, setExtractPath] = useState('');
  const [compression, setCompression] = useState<CompressionAlgorithm>('LZMA');
  const [encrypt, setEncrypt] = useState(false);
  const [password, setPassword] = useState('');
  const [isProcessing, setIsProcessing] = useState(false);
  const [progress, setProgress] = useState(0);
  const [logs, setLogs] = useState<LogEntry[]>([]);

  const addLog = (level: LogEntry['level'], message: string) => {
    setLogs((prev) => [...prev, { timestamp: new Date(), level, message }]);
  };

  const addFile = (file: string) => {
    if (file && !files.includes(file)) {
      setFiles([...files, file]);
      setNewFile('');
    }
  };

  const handleBrowseMultiple = async () => {
    const selected = await open({
      multiple: true,
      title: 'Select files to add to archive',
    });

    if (selected && Array.isArray(selected)) {
      const uniqueFiles = selected.filter(f => !files.includes(f));
      setFiles([...files, ...uniqueFiles]);
    }
  };

  const handleBrowseArchiveOutput = async () => {
    const selected = await save({
      title: 'Save archive as',
      defaultPath: 'archive.fva',
      filters: [{
        name: 'FileVault Archive',
        extensions: ['fva']
      }]
    });

    if (selected) {
      setArchivePath(selected);
    }
  };

  const removeFile = (index: number) => {
    setFiles(files.filter((_, i) => i !== index));
  };

  const handleCreate = async () => {
    if (files.length === 0) {
      addLog('error', 'Please add at least one file');
      return;
    }
    if (!archivePath) {
      addLog('error', 'Please specify archive output path');
      return;
    }
    if (encrypt && !password) {
      addLog('error', 'Please enter password for encryption');
      return;
    }

    setIsProcessing(true);
    setProgress(0);
    addLog('info', 'Creating archive...');

    try {
      const result = await createArchive({
        files,
        output: archivePath,
        compression,
        encrypt,
        password: encrypt ? password : undefined,
      });

      setProgress(100);

      if (result.success) {
        addLog('success', 'Archive created successfully!');
        addLog('info', result.stdout);
      } else {
        addLog('error', 'Archive creation failed: ' + (result.error || result.stderr));
      }
    } catch (error) {
      addLog('error', 'Unexpected error: ' + String(error));
    } finally {
      setIsProcessing(false);
    }
  };

  const handleExtract = async () => {
    if (!archivePath) {
      addLog('error', 'Please select archive file');
      return;
    }
    if (!extractPath) {
      addLog('error', 'Please specify extraction path');
      return;
    }

    setIsProcessing(true);
    setProgress(0);
    addLog('info', 'Extracting archive...');

    try {
      const result = await extractArchive({
        input: archivePath,
        output: extractPath,
        password: password || undefined,
      });

      setProgress(100);

      if (result.success) {
        addLog('success', 'Archive extracted successfully!');
        addLog('info', result.stdout);
      } else {
        addLog('error', 'Extraction failed: ' + (result.error || result.stderr));
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
        <h1 className="text-3xl font-bold">Archive Manager</h1>
        <p className="text-muted-foreground mt-2">
          Create or extract compressed archives with optional encryption
        </p>
      </div>

      <Card title="Mode Selection">
        <div className="flex gap-4">
          <Button
            variant={mode === 'create' ? 'default' : 'outline'}
            onClick={() => setMode('create')}
          >
            Create Archive
          </Button>
          <Button
            variant={mode === 'extract' ? 'default' : 'outline'}
            onClick={() => setMode('extract')}
          >
            Extract Archive
          </Button>
        </div>
      </Card>

      {mode === 'create' ? (
        <>
          <Card title="Files to Archive">
            <div className="space-y-4">
              <div className="flex gap-2">
                <Button onClick={handleBrowseMultiple} variant="default" className="flex-1">
                  <Files className="w-4 h-4 mr-2" />
                  Select Multiple Files
                </Button>
              </div>
              
              <div className="flex gap-2">
                <FilePicker
                  value={newFile}
                  onChange={setNewFile}
                  className="flex-1"
                  placeholder="Or select single file to add..."
                />
                <Button onClick={() => addFile(newFile)} variant="outline" size="icon">
                  <Plus className="w-4 h-4" />
                </Button>
              </div>

              {files.length > 0 && (
                <div className="space-y-2">
                  {files.map((file, index) => (
                    <div key={index} className="flex items-center justify-between p-2 bg-muted rounded">
                      <span className="text-sm truncate flex-1">{file}</span>
                      <Button
                        onClick={() => removeFile(index)}
                        variant="ghost"
                        size="icon"
                        className="h-6 w-6"
                      >
                        <X className="w-4 h-4" />
                      </Button>
                    </div>
                  ))}
                </div>
              )}
            </div>
          </Card>

          <Card title="Archive Settings">
            <div className="space-y-4">
              <div>
                <label className="block text-sm font-medium mb-2">Archive Output Path</label>
                <div className="flex gap-2">
                  <Input
                    type="text"
                    value={archivePath}
                    onChange={(e) => setArchivePath(e.target.value)}
                    placeholder="output.fva"
                    className="flex-1"
                  />
                  <Button
                    type="button"
                    variant="outline"
                    onClick={handleBrowseArchiveOutput}
                    disabled={isProcessing}
                  >
                    <FileUp className="w-4 h-4" />
                  </Button>
                </div>
              </div>

              <div>
                <label className="block text-sm font-medium mb-2">Compression</label>
                <Select
                  options={compressionOptions}
                  value={compression}
                  onChange={(e) => setCompression(e.target.value as CompressionAlgorithm)}
                />
                <p className="text-xs text-muted-foreground mt-1">
                  Compression level is automatically determined by algorithm
                </p>
              </div>

              <label className="flex items-center gap-2">
                <input
                  type="checkbox"
                  checked={encrypt}
                  onChange={(e) => setEncrypt(e.target.checked)}
                  className="rounded"
                />
                <span className="text-sm">Encrypt archive</span>
              </label>

              {encrypt && (
                <div>
                  <label className="block text-sm font-medium mb-2">Password</label>
                  <Input
                    type="password"
                    value={password}
                    onChange={(e) => setPassword(e.target.value)}
                    placeholder="Enter password"
                  />
                </div>
              )}
            </div>
          </Card>

          <Button onClick={handleCreate} disabled={isProcessing} size="lg">
            {isProcessing ? 'Creating...' : 'Create Archive'}
          </Button>
        </>
      ) : (
        <>
          <Card title="Extract Settings">
            <div className="space-y-4">
              <div>
                <label className="block text-sm font-medium mb-2">Archive File</label>
                <FilePicker value={archivePath} onChange={setArchivePath} accept={['fva']} />
              </div>

              <div>
                <label className="block text-sm font-medium mb-2">Extract to Directory</label>
                <FilePicker value={extractPath} onChange={setExtractPath} directory />
              </div>

              <div>
                <label className="block text-sm font-medium mb-2">Password (if encrypted)</label>
                <Input
                  type="password"
                  value={password}
                  onChange={(e) => setPassword(e.target.value)}
                  placeholder="Enter password"
                />
              </div>
            </div>
          </Card>

          <Button onClick={handleExtract} disabled={isProcessing} size="lg">
            {isProcessing ? 'Extracting...' : 'Extract Archive'}
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
