#ifndef PDF_FORMAT_H
#define PDF_FORMAT_H

#include <QString>

QString fmainHtml=
R"(<html>

    <head>
    <style>

.info-table {

width: 100%;
    border-collapse: collapse;

    margin-bottom: 30px;
}

.page-title {
    text-align: center;
    font-size: 20pt;
    font-weight: bold;
    margin-bottom: 50px;
}

.info-table td {
padding: 6px 10px;
vertical-align: top;
font-size: 10pt;
}

.test-table {
    border-collapse: collapse;

}

.test-table th,
    .test-table td {
border: 2px solid #000000;
padding: 8px;

}
</style>
</head>

<body>
<div class="page-title">Test Report</div>
%1
%2
</body>

</html>
)";

QString fheader=
    R"(
<table width=680 class="info-table">
<tr>
<td><strong>Aim:</strong> %1</td>
</tr>
<tr>
<td><strong>Product Name:</strong> %2</td>
<td><strong>Product ID:</strong> %3</td>
</tr>
<tr>
<td><strong>MCU:</strong> %4</td>
<td><strong>MCU ID:</strong> %5</td>
</tr>
<tr>
<td><strong>Hardware Version:</strong> %6</td>
<td><strong>Firmware Version:</strong> %7</td>
</tr>
</table>
)";

QString ftable=
    R"(
<table width=680 class="test-table">
<thead>
<tr>
<th class="test-table-c1">Tests</th>
<th width=90 class="test-table-c2">Status </th>
<th class="test-table-c3">Requirements </th>
</tr>
</thead>
<tbody>
%1
</tbody>
</table>
)";

QString frow=
    R"(
<tr>
<td>%1</td>
<td>%2</td>
<td>%3</td>
</tr>
)";
#endif // PDF_FORMAT_H
