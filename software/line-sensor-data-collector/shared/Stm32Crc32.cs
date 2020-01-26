namespace line_sensor.data_collector.shared
{
    public class Stm32Crc32
    {
        public Stm32Crc32(uint initialValue, uint polynomial)
        {
            this.polynomial = polynomial;
            this.previousValue = initialValue;
        }

        public void Encode(uint currentValue)
        {
            uint crc = this.previousValue ^ currentValue;

            for (uint i = 0; i < 32; ++i)
            {
                if ((crc & msb) != 0)
                {
                    crc = (crc << 1) ^ this.polynomial;
                }
                else
                {
                    crc = crc << 1;
                }
            }

            this.previousValue = crc;
        }

        public uint GetValue()
        {
            return this.previousValue;
        }

        private uint previousValue;
        private readonly uint polynomial;

        private const uint msb = 1u << 31;
    }
}
