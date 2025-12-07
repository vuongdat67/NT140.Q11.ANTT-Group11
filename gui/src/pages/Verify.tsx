import { useState } from 'react';
import { Card } from '../components/Card';
import { FilePicker } from '../components/FilePicker';
import { Button } from '../components/Button';
import { LogPanel } from '../components/LogPanel';
import { verifySignature } from '../lib/cli';
import type { LogEntry } from '../types';
import { CheckCircle2, XCircle } from 'lucide-react';

export function Verify() {
  const [inputFile, setInputFile] = useState('');
  const [signatureFile, setSignatureFile] = useState('');
  const [publicKey, setPublicKey] = useState('');
  const [isProcessing, setIsProcessing] = useState(false);
  const [verificationResult, setVerificationResult] = useState<boolean | null>(null);
  const [logs, setLogs] = useState<LogEntry[]>([]);

  const addLog = (level: LogEntry['level'], message: string) => {
    setLogs((prev) => [...prev, { timestamp: new Date(), level, message }]);
  };

  const handleVerify = async () => {
    if (!inputFile || !signatureFile || !publicKey) {
      addLog('error', 'Please fill all required fields');
      return;
    }

    setIsProcessing(true);
    setVerificationResult(null);
    addLog('info', 'Verifying signature...');

    try {
      const result = await verifySignature({
        input: inputFile,
        signature: signatureFile,
        publicKey,
      });

      if (result.success) {
        setVerificationResult(true);
        addLog('success', 'Signature is VALID!');
        addLog('info', result.stdout);
      } else {
        setVerificationResult(false);
        addLog('error', 'Signature is INVALID or verification failed');
        addLog('error', result.stderr);
      }
    } catch (error) {
      setVerificationResult(false);
      addLog('error', 'Unexpected error: ' + String(error));
    } finally {
      setIsProcessing(false);
    }
  };

  return (
    <div className="space-y-6">
      <div>
        <h1 className="text-3xl font-bold">Verify Signature</h1>
        <p className="text-muted-foreground mt-2">
          Verify digital signatures to authenticate files
        </p>
      </div>

      <Card title="File Selection">
        <div className="space-y-4">
          <div>
            <label className="block text-sm font-medium mb-2">Input File</label>
            <FilePicker value={inputFile} onChange={setInputFile} />
          </div>

          <div>
            <label className="block text-sm font-medium mb-2">Signature File</label>
            <FilePicker value={signatureFile} onChange={setSignatureFile} accept={['sig', 'sign']} />
            <p className="text-xs text-muted-foreground mt-1">
              Accepts .sig and .sign files
            </p>
          </div>

          <div>
            <label className="block text-sm font-medium mb-2">Public Key</label>
            <FilePicker value={publicKey} onChange={setPublicKey} accept={['pem', 'pub']} />
            <p className="text-xs text-muted-foreground mt-1">
              Use .pub file generated from Keygen page (PEM format)
            </p>
          </div>
        </div>
      </Card>

      <div className="flex gap-4">
        <Button onClick={handleVerify} disabled={isProcessing} size="lg">
          {isProcessing ? 'Verifying...' : 'Verify Signature'}
        </Button>
        <Button
          variant="outline"
          onClick={() => {
            setInputFile('');
            setSignatureFile('');
            setPublicKey('');
            setVerificationResult(null);
            setLogs([]);
          }}
          disabled={isProcessing}
        >
          Clear
        </Button>
      </div>

      {verificationResult !== null && (
        <Card title="Verification Result">
          <div className="flex items-center gap-4">
            {verificationResult ? (
              <>
                <CheckCircle2 className="w-12 h-12 text-green-500" />
                <div>
                  <h3 className="text-xl font-bold text-green-500">Signature Valid</h3>
                  <p className="text-sm text-muted-foreground">
                    The file has been signed by the holder of the private key
                  </p>
                </div>
              </>
            ) : (
              <>
                <XCircle className="w-12 h-12 text-red-500" />
                <div>
                  <h3 className="text-xl font-bold text-red-500">Signature Invalid</h3>
                  <p className="text-sm text-muted-foreground">
                    The signature does not match or the file has been tampered with
                  </p>
                </div>
              </>
            )}
          </div>
        </Card>
      )}

      {logs.length > 0 && <LogPanel logs={logs} />}
    </div>
  );
}
