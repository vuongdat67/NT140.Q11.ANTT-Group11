import { useState } from 'react';
import { Card } from '../components/Card';
import { Select } from '../components/Select';
import { Button } from '../components/Button';
import { Input } from '../components/Input';
import { LogPanel } from '../components/LogPanel';
import { benchmarkAlgorithm } from '../lib/cli';
import type { LogEntry } from '../types';
import { Zap, Award, AlertCircle } from 'lucide-react';

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
    let currentSection = '';
    let headers: string[] = [];
    
    for (let i = 0; i < lines.length; i++) {
      const line = lines[i].trim();
      
      // Skip empty lines and separators
      if (!line || line.startsWith('===') || line.startsWith('---') || line.startsWith('+')) {
        continue;
      }
      
      // Detect section headers
      if (line.includes('SYMMETRIC') || line.includes('ASYMMETRIC') || line.includes('POST-QUANTUM') || 
          line.includes('KEY DERIVATION') || line.includes('COMPRESSION') || line.includes('HASH')) {
        currentSection = line;
        headers = [];
        continue;
      }
      
      // Parse table rows with | separators
      if (line.includes('|')) {
        const parts = line.split('|').map(p => p.trim()).filter(p => p && !p.match(/^[-+]+$/));
        
        // Detect header row (usually contains column names)
        if (parts.some(p => /^(Algorithm|Variant|Operation|Encrypt|Decrypt|KeyGen|Encaps|Decaps|Sign|Verify|Compress|Decompress|Time|Throughput|Security|Notes|Digest|Rate|Memory|Ratio)$/i.test(p))) {
          headers = parts;
          continue;
        }
        
        // Parse data rows
        if (parts.length >= 2 && headers.length > 0) {
          const algorithm = parts[0] || '';
          
          // Skip if this looks like a header or separator
          if (!algorithm || algorithm.match(/^(Algorithm|Variant|Operation)$/i)) {
            continue;
          }
          
          // Determine operation type based on section and columns
          let operation = '';
          let time = '';
          let throughput = '';
          
          // Map headers to indices
          const headerMap: { [key: string]: number } = {};
          headers.forEach((h, idx) => {
            headerMap[h.toLowerCase()] = idx;
          });
          
          // Extract data based on section type
          if (currentSection.includes('SYMMETRIC') || currentSection.includes('AEAD') || currentSection.includes('Block Cipher')) {
            // Symmetric: Encrypt, Decrypt columns
            if (headerMap['encrypt'] !== undefined && parts[headerMap['encrypt']]) {
              operation = 'Encrypt';
              throughput = parts[headerMap['encrypt']];
            }
            if (headerMap['decrypt'] !== undefined && parts[headerMap['decrypt']]) {
              // Add decrypt as separate row if both exist
              if (operation === 'Encrypt') {
                parsed.push({
                  algorithm,
                  operation: 'Encrypt',
                  time: '-',
                  throughput: parts[headerMap['encrypt']] || '-',
                  speed: parts[headerMap['encrypt']] || '-',
                });
              }
              operation = 'Decrypt';
              throughput = parts[headerMap['decrypt']];
            }
          } else if (currentSection.includes('ASYMMETRIC')) {
            // Asymmetric: KeyGen, Encrypt, Decrypt columns
            if (headerMap['keygen'] !== undefined && parts[headerMap['keygen']]) {
              operation = 'KeyGen';
              time = parts[headerMap['keygen']];
            } else if (headerMap['encrypt'] !== undefined && parts[headerMap['encrypt']]) {
              operation = 'Encrypt';
              time = parts[headerMap['encrypt']];
            } else if (headerMap['decrypt'] !== undefined && parts[headerMap['decrypt']]) {
              operation = 'Decrypt';
              time = parts[headerMap['decrypt']];
            }
          } else if (currentSection.includes('POST-QUANTUM') || currentSection.includes('Kyber') || currentSection.includes('Dilithium')) {
            // Post-quantum: KeyGen, Encaps, Decaps, Sign, Verify
            if (headerMap['keygen'] !== undefined && parts[headerMap['keygen']]) {
              operation = 'KeyGen';
              time = parts[headerMap['keygen']];
            } else if (headerMap['encaps'] !== undefined && parts[headerMap['encaps']]) {
              operation = 'Encaps';
              time = parts[headerMap['encaps']];
            } else if (headerMap['decaps'] !== undefined && parts[headerMap['decaps']]) {
              operation = 'Decaps';
              time = parts[headerMap['decaps']];
            } else if (headerMap['sign'] !== undefined && parts[headerMap['sign']]) {
              operation = 'Sign';
              time = parts[headerMap['sign']];
            } else if (headerMap['verify'] !== undefined && parts[headerMap['verify']]) {
              operation = 'Verify';
              time = parts[headerMap['verify']];
            } else if (headerMap['encrypt'] !== undefined && parts[headerMap['encrypt']]) {
              operation = 'Encrypt';
              throughput = parts[headerMap['encrypt']];
            } else if (headerMap['decrypt'] !== undefined && parts[headerMap['decrypt']]) {
              operation = 'Decrypt';
              throughput = parts[headerMap['decrypt']];
            }
          } else if (currentSection.includes('KEY DERIVATION')) {
            // KDF: Time, Rate columns
            if (headerMap['time'] !== undefined && parts[headerMap['time']]) {
              operation = 'KDF';
              time = parts[headerMap['time']];
            }
            if (headerMap['rate'] !== undefined && parts[headerMap['rate']]) {
              throughput = parts[headerMap['rate']] + '/s';
            }
          } else if (currentSection.includes('COMPRESSION')) {
            // Compression: Compress, Decompress columns
            if (headerMap['compress'] !== undefined && parts[headerMap['compress']]) {
              operation = 'Compress';
              throughput = parts[headerMap['compress']];
            }
            if (headerMap['decompress'] !== undefined && parts[headerMap['decompress']]) {
              if (operation === 'Compress') {
                parsed.push({
                  algorithm,
                  operation: 'Compress',
                  time: '-',
                  throughput: parts[headerMap['compress']] || '-',
                  speed: parts[headerMap['compress']] || '-',
                });
              }
              operation = 'Decompress';
              throughput = parts[headerMap['decompress']];
            }
          } else if (currentSection.includes('HASH')) {
            // Hash: Throughput column
            if (headerMap['throughput'] !== undefined && parts[headerMap['throughput']]) {
              operation = 'Hash';
              throughput = parts[headerMap['throughput']];
            }
          }
          
          if (algorithm && operation) {
            parsed.push({
              algorithm,
              operation,
              time: time || '-',
              throughput: throughput || '-',
              speed: throughput || time || '-',
            });
          }
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
