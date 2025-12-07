import { useState } from 'react';
import { Card } from '../components/Card';
import { Select } from '../components/Select';
import { Button } from '../components/Button';
import { Input } from '../components/Input';
import { LogPanel } from '../components/LogPanel';
import { benchmarkAlgorithm } from '../lib/cli';
import type { LogEntry } from '../types';
import { Zap, Award, AlertCircle, Loader2, TrendingUp } from 'lucide-react';

const algorithmOptions = [
  { value: 'all', label: 'All Algorithms' },
  { value: 'aes-256-gcm', label: 'AES-256-GCM' },
  { value: 'chacha20-poly1305', label: 'ChaCha20-Poly1305' },
  { value: 'rsa-4096', label: 'RSA-4096' },
  { value: 'ecc-p521', label: 'ECC-P521' },
  { value: 'kyber-1024-hybrid', label: 'Kyber-1024-Hybrid' },
];

const categoryOptions = [
  { value: '', label: 'All Categories' },
  { value: 'symmetric', label: 'Symmetric Only' },
  { value: 'asymmetric', label: 'Asymmetric Only' },
  { value: 'pqc', label: 'Post-Quantum Only' },
  { value: 'hash', label: 'Hash Functions Only' },
  { value: 'kdf', label: 'Key Derivation Only' },
  { value: 'compression', label: 'Compression Only' },
];

const sizeOptions = [
  { value: '1048576', label: '1 MB' },
  { value: '10485760', label: '10 MB' },
  { value: '104857600', label: '100 MB' },
];

interface BenchmarkResult {
  algorithm: string;
  operation: string;
  time: string;
  throughput: string;
  speed: string;
}

export function Benchmark() {
  const [algorithm, setAlgorithm] = useState('all');
  const [category, setCategory] = useState('');
  const [size, setSize] = useState('1048576');
  const [iterations, setIterations] = useState('5');
  const [isProcessing, setIsProcessing] = useState(false);
  const [logs, setLogs] = useState<LogEntry[]>([]);
  const [results, setResults] = useState<BenchmarkResult[]>([]);
  const [error, setError] = useState('');

  const addLog = (level: LogEntry['level'], message: string) => {
    setLogs((prev) => [...prev, { timestamp: new Date(), level, message }]);
  };

  const parseBenchmarkOutput = (output: string): BenchmarkResult[] => {
    const lines = output.split('\n');
    const parsed: BenchmarkResult[] = [];
    
    for (const line of lines) {
      // Try to parse different benchmark output formats
      // Format 1: "Algorithm: AES-256-GCM | Operation: Encrypt | Time: 123.45ms | Throughput: 8.10 MB/s"
      // Format 2: "AES-256-GCM: Encrypt: 123.45ms (8.10 MB/s)"
      // Format 3: Table format with | separators
      
      if (line.includes('|')) {
        const parts = line.split('|').map(p => p.trim());
        if (parts.length >= 3) {
          const algorithm = parts.find(p => p.includes('Algorithm') || /^[A-Z]/.test(p))?.replace('Algorithm:', '').trim() || '';
          const operation = parts.find(p => p.includes('Operation') || p.includes('Encrypt') || p.includes('Decrypt'))?.replace('Operation:', '').trim() || '';
          const time = parts.find(p => p.includes('Time') || p.includes('ms') || p.includes('s'))?.replace('Time:', '').trim() || '';
          const throughput = parts.find(p => p.includes('Throughput') || p.includes('MB/s') || p.includes('GB/s'))?.replace('Throughput:', '').trim() || '';
          
          if (algorithm || operation) {
            parsed.push({
              algorithm: algorithm || 'Unknown',
              operation: operation || 'Unknown',
              time: time || '-',
              throughput: throughput || '-',
              speed: throughput || '-',
            });
          }
        }
      } else if (line.includes(':') && (line.includes('ms') || line.includes('MB/s'))) {
        // Format: "AES-256-GCM: Encrypt: 123.45ms (8.10 MB/s)"
        const match = line.match(/([^:]+):\s*([^:]+):\s*([\d.]+(?:ms|s))\s*\(([\d.]+\s*(?:MB|GB)\/s)\)/);
        if (match) {
          parsed.push({
            algorithm: match[1].trim(),
            operation: match[2].trim(),
            time: match[3].trim(),
            throughput: match[4].trim(),
            speed: match[4].trim(),
          });
        }
      }
    }
    
    return parsed;
  };

  const handleBenchmark = async () => {
    setIsProcessing(true);
    setLogs([]);
    setResults([]);
    setError('');

    const options: any = {
      size: parseInt(size),
      iterations: parseInt(iterations),
    };

    if (algorithm !== 'all') {
      options.algorithm = algorithm;
    }

    if (category) {
      options[category] = true;
    }

    addLog('info', 'Starting benchmark...');
    addLog('info', `Configuration: Size=${size} bytes, Iterations=${iterations}`);

    try {
      const result = await benchmarkAlgorithm(options, (log) => {
        addLog(log.level, log.message);
      });
      
      if (result.success && result.stdout) {
        addLog('success', 'Benchmark completed successfully');
        const parsed = parseBenchmarkOutput(result.stdout);
        if (parsed.length > 0) {
          setResults(parsed);
        } else {
          // If parsing fails, show raw output in logs
          addLog('info', 'Raw benchmark output:');
          result.stdout.split('\n').forEach(line => {
            if (line.trim()) addLog('info', line);
          });
        }
      } else {
        const errorMsg = result.error || result.stderr || 'Benchmark failed';
        setError(errorMsg);
        addLog('error', errorMsg);
      }
    } catch (err) {
      const errorMsg = err instanceof Error ? err.message : 'Unknown error';
      setError(errorMsg);
      addLog('error', errorMsg);
    } finally {
      setIsProcessing(false);
    }
  };

  return (
    <div className="space-y-6">
      <div>
        <h1 className="text-3xl font-bold">Benchmark</h1>
        <p className="text-base-content/70 mt-2">
          Performance testing for cryptographic algorithms
        </p>
      </div>

      <Card title="Benchmark Configuration" description="Configure benchmark parameters and run performance tests">
        <div className="space-y-4">
          <div className="grid grid-cols-1 md:grid-cols-2 gap-4">
            <div>
              <label className="label">
                <span className="label-text font-medium">Algorithm</span>
              </label>
              <Select
                options={algorithmOptions}
                value={algorithm}
                onChange={(e) => setAlgorithm(e.target.value)}
              />
            </div>

            <div>
              <label className="label">
                <span className="label-text font-medium">Category Filter</span>
              </label>
              <Select
                options={categoryOptions}
                value={category}
                onChange={(e) => setCategory(e.target.value)}
              />
            </div>
          </div>

          <div className="grid grid-cols-1 md:grid-cols-2 gap-4">
            <div>
              <label className="label">
                <span className="label-text font-medium">Data Size</span>
              </label>
              <Select
                options={sizeOptions}
                value={size}
                onChange={(e) => setSize(e.target.value)}
              />
            </div>

            <div>
              <label className="label">
                <span className="label-text font-medium">Iterations</span>
              </label>
              <Input
                type="number"
                min="1"
                max="100"
                value={iterations}
                onChange={(e) => setIterations(e.target.value)}
                placeholder="Number of iterations"
              />
            </div>
          </div>

          <div className="pt-2">
            <Button
              onClick={handleBenchmark}
              disabled={isProcessing}
              loading={isProcessing}
              size="lg"
              className="w-full"
            >
              {!isProcessing && <Zap className="w-5 h-5 mr-2" />}
              {isProcessing ? 'Running Benchmark...' : 'Start Benchmark'}
            </Button>
          </div>
        </div>
      </Card>

      {error && (
        <div className="alert alert-error shadow-lg">
          <AlertCircle className="w-6 h-6" />
          <div className="flex-1">
            <h3 className="font-bold">Error</h3>
            <div className="text-sm">
              <pre className="whitespace-pre-wrap">{error}</pre>
            </div>
          </div>
        </div>
      )}

      {logs.length > 0 && (
        <Card title="Benchmark Logs">
          <LogPanel logs={logs} maxHeight="400px" />
        </Card>
      )}

      {results.length > 0 && (
        <Card title="Benchmark Results" className="shadow-lg">
          <div className="space-y-4">
            <div className="flex items-center gap-2 text-success">
              <Award className="w-6 h-6" />
              <span className="font-semibold">Benchmark completed successfully</span>
            </div>
            
            <div className="overflow-x-auto">
              <table className="table table-zebra w-full">
                <thead>
                  <tr>
                    <th>Algorithm</th>
                    <th>Operation</th>
                    <th>Time</th>
                    <th>Throughput</th>
                    <th>Speed</th>
                  </tr>
                </thead>
                <tbody>
                  {results.map((result, index) => (
                    <tr key={index}>
                      <td className="font-mono font-semibold">{result.algorithm}</td>
                      <td>{result.operation}</td>
                      <td className="font-mono">{result.time}</td>
                      <td className="font-mono text-success">{result.throughput}</td>
                      <td className="font-mono">{result.speed}</td>
                    </tr>
                  ))}
                </tbody>
              </table>
            </div>
          </div>
        </Card>
      )}
    </div>
  );
}
