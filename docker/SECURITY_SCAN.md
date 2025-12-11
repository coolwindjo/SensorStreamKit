# Docker Security Scan Results

## Commands
% docker build --platform linux/amd64 -f docker/Dockerfile.dev -t sensorstreamkit-dev --target dev .
% docker scout quickview

## Scan Date
2025-12-11

## Image Scanned
- **Image**: `sensorstreamkit-dev:latest`
- **Digest**: `462801ba6f70`
- **Base Image**: `ubuntu:24.04`

## Vulnerability Summary
| Severity | Count |
|----------|-------|
| Critical | 0     |
| High     | 4     |
| Medium   | 1165  |
| Low      | 40    |
| **Total** | **1209** |

## Base Image Information
- **Current Base Image**: `ubuntu:24.04`
- **Updated Base Image Available**: `ubuntu:26.04`
  - Would reduce: 2 Medium, 5 Low vulnerabilities

## Decision
**Keep Ubuntu 24.04 as the base image** - The current version is intentionally maintained for compatibility and stability reasons.

## Notes
- Docker Scout version: 1.18.3 (1.18.4 available)
- Total packages indexed: 467
- Base image was auto-detected. For more accurate results, consider building images with max-mode provenance attestations.
- Review [docs.docker.com](https://docs.docker.com) for more information.

## Recommendations
1. Monitor the 4 High severity vulnerabilities and address if they affect production use
2. Review Medium severity vulnerabilities periodically
3. Consider updating base image when Ubuntu 24.04 LTS support ends (April 2029)
4. Run `docker scout quickview` regularly to track vulnerability changes

