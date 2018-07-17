void TressFX_DefaultRead(void* ptr, AMD::uint size, EI_Stream* pFile) {
  FILE* fp = reinterpret_cast<FILE*>(pFile);
  fread(ptr, size, 1, fp);
}

void TressFX_DefaultSeek(EI_Stream* pFile, AMD::uint offset) {
  FILE* fp = reinterpret_cast<FILE*>(pFile);
  fseek(fp, offset, SEEK_SET);
}
