using System;
using NUnit.Framework;
using RagelVsExtension;
using System.Reflection;
using System.IO;
using System.Linq;
using System.Text;

namespace RagelVsExtensionTests
{
    [TestFixture]
    public class GeneratorTests
    {
        [Test]
        [TestCase("sample")]
        public void GivenValidInput_Generate_ShouldReturnCSCode(string filename)
        {
            // Arrange
            var generator = new CodeGenerator();
            var content = ReadAllTextFromEmbeddedResource(filename + ".rl");
            var expected = ReadAllTextFromEmbeddedResource(filename + ".cs");

            // Act
            var actual = generator.Generate(filename, content, new DateTime(2016,1,1,8,0,0));

            // Assert
            Assert.AreEqual(FixWhitespaces(expected), FixWhitespaces(Encoding.UTF8.GetString(actual)));
        }

        [Test]
        [TestCase("invalid_sample")]
        public void GivenInvalidInput_Generate_ShouldReturnCSCodeWithErrorDirective(string filename)
        {
            // Arrange
            var generator = new CodeGenerator();
            var content = ReadAllTextFromEmbeddedResource(filename + ".rl");
            var expected = ReadAllTextFromEmbeddedResource(filename + ".cs");

            // Act
            var actual = generator.Generate(filename, content, new DateTime(2016, 1, 1, 8, 0, 0));

            // Assert
            var actualStr = Encoding.UTF8.GetString(actual);
            Assert.AreEqual(FixWhitespaces(expected), FixWhitespaces(Encoding.UTF8.GetString(actual)));
        }

        [Test]
        [TestCase("invalid_sample", "sample")]
        public void GivenInvalidAndThenValidInput_Generate_ShouldReturnCSCodeWithErrorDirectiveAndThenProperCSCode(
            string invalid, string valid)
        {
            // Arrange
            var generator = new CodeGenerator();
            var invalidContent = ReadAllTextFromEmbeddedResource(invalid + ".rl");
            var invalidExpected = ReadAllTextFromEmbeddedResource(invalid + ".cs");
            var validContent = ReadAllTextFromEmbeddedResource(valid + ".rl");
            var validExpected = ReadAllTextFromEmbeddedResource(valid + ".cs");

            // Act
            var result1 = generator.Generate(invalid, invalidContent, new DateTime(2016, 1, 1, 8, 0, 0));
            var result2 = generator.Generate(valid, validContent, new DateTime(2016, 1, 1, 8, 0, 0));

            // Assert
            Assert.AreEqual(FixWhitespaces(invalidExpected), FixWhitespaces(Encoding.UTF8.GetString(result1)));
            Assert.AreEqual(FixWhitespaces(validExpected), FixWhitespaces(Encoding.UTF8.GetString(result2)));
        }

        [Test]
        [TestCase("sample")]
        public void GivenValidInput_Generate2Times_ShouldReturnCSCodeBothTimes(string filename)
        {
            // Arrange
            var generator = new CodeGenerator();
            var content = ReadAllTextFromEmbeddedResource(filename + ".rl");
            var expected = ReadAllTextFromEmbeddedResource(filename + ".cs");

            // Act
            var actual = generator.Generate(filename, content, new DateTime(2016, 1, 1, 8, 0, 0));
            var another = generator.Generate(filename, content, new DateTime(2016, 1, 1, 8, 0, 0));

            // Assert
            Assert.AreEqual(FixWhitespaces(expected), FixWhitespaces(Encoding.UTF8.GetString(actual)));
            Assert.AreEqual(FixWhitespaces(expected), FixWhitespaces(Encoding.UTF8.GetString(another)));
        }

        private static string FixWhitespaces(string str)
        {
            str = str.Replace("\r\n", "\n")
                .Replace("\t", " ");
            int previousLength;
            do
            {
                previousLength = str.Length;
                str = str.Replace("  ", " ")
                    .Replace("\n\n", "\n")
                    .Replace("\n ", "\n")
                    .Replace(" \n", "\n")
                    .Replace(" [", "[")
                    .Replace("\n{", "{")
                    .Replace(" {", "{")
                    .Replace(" }", "}")
                    .Replace("{ ", "{")
                    .Replace("( ", "(")
                    .Replace(") ", ")")
                    .Replace(" )", ")")
                    .Replace(" - ", "-")
                    .Replace(" + ", "+")
                    .Replace(" << ", "<<")
                    .Replace(" >> ", ">>")
                    .Replace(">> ", ">>")
                    ;
            } while (str.Length != previousLength);
            return str;
        }

        private static string ReadAllTextFromEmbeddedResource(string filename)
        {
            var assembly = Assembly.GetExecutingAssembly();
            var path = FindResourcePath(assembly, filename);
            using (var stream = assembly.GetManifestResourceStream(path))
            using (var reader = new StreamReader(stream))
            {
                return reader.ReadToEnd();
            }
        }

        private static string FindResourcePath(Assembly assembly, string filename)
        {
            var result = assembly.GetManifestResourceNames()
                .FirstOrDefault(name => name.EndsWith("." + filename));
            if (result == null)
            {
                throw new Exception(string.Format(
                    "Unable to find embedded resource '{0}'", filename));
            }
            return result;
        }
    }
}
