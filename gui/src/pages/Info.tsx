import { useState } from 'react';
import { Card } from '../components/Card';
import { FilePicker } from '../components/FilePicker';
import { Button } from '../components/Button';
import { LogPanel } from '../components/LogPanel';
import { getFileInfo } from '../lib/cli';
import type { LogEntry } from '../types';
import { Info as InfoIcon, Shield, Zap, Package, Lock } from 'lucide-react';

interface FileInfoData {
  file: string;
  totalSize: string;
  formatVersion: string;
  encryption: string;
  kdf: string;
  compression: string;
  headerSize: string;
  salt: string;
  nonce: string;
  authTag: string;
  encryptedData: string;
  metadataOverhead: string;
  payloadRatio: string;
}

export function Info() {
  const [inputFile, setInputFile] = useState('');
  const [verbose, setVerbose] = useState(false);
  const [isProcessing, setIsProcessing] = useState(false);
  const [fileInfo, setFileInfo] = useState<FileInfoData | null>(null);
  const [logs, setLogs] = useState<LogEntry[]>([]);
  const [error, setError] = useState('');

  const addLog = (level: LogEntry['level'], message: string) => {
    setLogs((prev) => [...prev, { timestamp: new Date(), level, message }]);
  };

  const parseFileInfo = (output: string): FileInfoData | null => {
    try {
      const lines = output.split('\n');
      const data: any = {};
      
      for (const line of lines) {
        if (line.includes('File') && line.includes(':') && !line.includes('Information')) {
          const parts = line.split(':');
          if (parts.length >= 2) data.file = parts.slice(1).join(':').trim();
        }
        if (line.includes('Total Size')) data.totalSize = line.split(':')[1]?.trim() || '';
        if (line.includes('Format Version')) data.formatVersion = line.split(':')[1]?.trim() || '';
        if (line.includes('Encryption') && !line.includes('Details')) data.encryption = line.split(':')[1]?.trim() || '';
        if (line.includes('Key Derivation')) data.kdf = line.split(':')[1]?.trim() || '';
        if (line.includes('Compression') && line.includes(':') && !line.includes('Encrypted')) data.compression = line.split(':')[1]?.trim() || '';
        if (line.includes('Header Size')) data.headerSize = line.split(':')[1]?.trim() || '';
        if (line.includes('Salt')) data.salt = line.split(':')[1]?.trim() || '';
        if (line.includes('Nonce')) data.nonce = line.split(':')[1]?.trim() || '';
        if (line.includes('Auth Tag')) data.authTag = line.split(':')[1]?.trim() || '';
        if (line.includes('Encrypted Data')) data.encryptedData = line.split(':')[1]?.trim() || '';
        if (line.includes('Metadata Overhead')) data.metadataOverhead = line.split(':')[1]?.trim() || '';
        if (line.includes('Payload Ratio')) data.payloadRatio = line.split(':')[1]?.trim() || '';
      }
      
      return data as FileInfoData;
    } catch {
      return null;
    }
  };

  const handleGetInfo = async () => {
    if (!inputFile) {
      addLog('error', 'Please select an encrypted file');
      setError('Please select an encrypted file');
      return;
    }

    setIsProcessing(true);
    setError('');
    setFileInfo(null);
    setLogs([]);
    addLog('info', 'Reading file information...');
    addLog('info', `File: ${inputFile}`);
    addLog('info', `Verbose mode: ${verbose ? 'enabled' : 'disabled'}`);

    try {
      const result = await getFileInfo(
        { input: inputFile, verbose },
        (log) => {
          addLog(log.level, log.message);
        }
      );

      if (result.success && result.stdout) {
        addLog('success', 'File information retrieved successfully');
        // Log all output lines
        result.stdout.split('\n').forEach((line) => {
          if (line.trim()) {
            addLog('info', line);
          }
        });
        
        const parsed = parseFileInfo(result.stdout);
        if (parsed) {
          setFileInfo(parsed);
          addLog('success', 'File information parsed successfully');
        } else {
          addLog('warning', 'Failed to parse file information, showing raw output');
          setError('Failed to parse file information');
        }
      } else {
        const errorMsg = result.error || result.stderr || 'Failed to get file info';
        addLog('error', errorMsg);
        setError(errorMsg);
      }
    } catch (error) {
      const errorMsg = error instanceof Error ? error.message : 'Unknown error';
      addLog('error', errorMsg);
      setError(errorMsg);
    } finally {
      setIsProcessing(false);
    }
  };

  return (
    <div className="space-y-6">
      <div>
        <h1 className="text-3xl font-bold">File Info</h1>
        <p className="text-base-content/70 mt-2">
          Display information about encrypted files
        </p>
      </div>

      <Card title="File Information">
        <div className="space-y-4">
          <div>
            <label className="block text-sm font-medium mb-2">Encrypted File</label>
            <FilePicker
              value={inputFile}
              onChange={setInputFile}
              placeholder="Select encrypted file (.fvlt)"
              accept={['fvlt']}
            />
          </div>

          <div>
            <label className="flex items-center gap-2">
              <input
                type="checkbox"
                className="checkbox checkbox-primary"
                checked={verbose}
                onChange={(e) => setVerbose(e.target.checked)}
              />
              <span className="text-sm">Show detailed information (verbose)</span>
            </label>
          </div>

          <Button
            onClick={handleGetInfo}
            disabled={!inputFile || isProcessing}
            className="w-full"
          >
            <InfoIcon className="w-4 h-4 mr-2" />
            {isProcessing ? 'Reading...' : 'Get File Info'}
          </Button>

          {error && (
            <div className="alert alert-error">
              <span>{error}</span>
            </div>
          )}
        </div>
      </Card>

      {logs.length > 0 && (
        <Card title="Logs">
          <LogPanel logs={logs} maxHeight="400px" />
        </Card>
      )}

      {fileInfo && (
        <>
          <div className="grid grid-cols-1 md:grid-cols-2 gap-4">
            <Card title="📄 File Details">
              <div className="space-y-3 text-sm">
                <div className="flex justify-between">
                  <span className="text-base-content/70">File:</span>
                  <span className="font-mono text-xs break-all max-w-[250px]">{fileInfo.file}</span>
                </div>
                <div className="flex justify-between">
                  <span className="text-base-content/70">Total Size:</span>
                  <span className="badge badge-primary">{fileInfo.totalSize}</span>
                </div>
                <div className="flex justify-between">
                  <span className="text-base-content/70">Format:</span>
                  <span className="badge badge-secondary">{fileInfo.formatVersion}</span>
                </div>
              </div>
            </Card>

            <Card title="🔐 Encryption">
              <div className="space-y-3 text-sm">
                <div className="flex items-center gap-2">
                  <Lock className="w-4 h-4 text-primary" />
                  <span className="text-base-content/70">Algorithm:</span>
                  <span className="badge badge-accent">{fileInfo.encryption}</span>
                </div>
                <div className="flex items-center gap-2">
                  <Shield className="w-4 h-4 text-success" />
                  <span className="text-base-content/70">KDF:</span>
                  <span className="badge badge-success">{fileInfo.kdf}</span>
                </div>
                <div className="flex items-center gap-2">
                  <Package className="w-4 h-4 text-info" />
                  <span className="text-base-content/70">Compression:</span>
                  <span className="badge badge-info">{fileInfo.compression}</span>
                </div>
              </div>
            </Card>
          </div>

          <Card title="🔒 Technical Details">
            <div className="grid grid-cols-2 md:grid-cols-4 gap-4 text-sm">
              <div className="stat bg-base-200 rounded-lg">
                <div className="stat-title text-xs">Header Size</div>
                <div className="stat-value text-lg">{fileInfo.headerSize}</div>
              </div>
              <div className="stat bg-base-200 rounded-lg">
                <div className="stat-title text-xs">Salt</div>
                <div className="stat-value text-lg">{fileInfo.salt}</div>
              </div>
              <div className="stat bg-base-200 rounded-lg">
                <div className="stat-title text-xs">Nonce</div>
                <div className="stat-value text-lg">{fileInfo.nonce}</div>
              </div>
              <div className="stat bg-base-200 rounded-lg">
                <div className="stat-title text-xs">Auth Tag</div>
                <div className="stat-value text-lg">{fileInfo.authTag}</div>
              </div>
            </div>
          </Card>

          <Card title="📊 Statistics">
            <div className="stats stats-vertical lg:stats-horizontal shadow w-full bg-base-100">
              <div className="stat">
                <div className="stat-figure text-primary">
                  <Zap className="w-8 h-8" />
                </div>
                <div className="stat-title">Encrypted Data</div>
                <div className="stat-value text-primary">{fileInfo.encryptedData}</div>
              </div>

              <div className="stat">
                <div className="stat-title">Metadata Overhead</div>
                <div className="stat-value text-secondary">{fileInfo.metadataOverhead}</div>
                <div className="stat-desc">Space used by headers</div>
              </div>

              <div className="stat">
                <div className="stat-title">Payload Ratio</div>
                <div className="stat-value text-accent">{fileInfo.payloadRatio}</div>
                <div className="stat-desc">Actual data efficiency</div>
              </div>
            </div>
          </Card>
        </>
      )}
    </div>
  );
}
